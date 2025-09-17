#include "Client.hpp"

bool Client::isCgi()
{
	string path =_config->full_path;
	if (path.rfind(".php") == path.size() - 4)
		return true;
	else if (path.rfind(".py") == path.size() - 3)
		return true;
	else if (path.find("/cgi-bin/") != std::string::npos)
 		return true;
	return false;
}

bool Client::isQueryStringValid()
{
	const std::string &qs = _config->query_string;
	if (qs.empty())
		return true;

	bool inKey = true;
	bool hasEqual = false;
	char prev = 0;

	for (size_t i = 0; i < qs.size(); ++i)
	{
		char c = qs[i];

		if (isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~' || c == '+') {
		}
		else if (c == '%')
		{
			if (i + 2 >= qs.size() || !isxdigit(qs[i+1]) || !isxdigit(qs[i+2]))
				return false;
			i += 2;
		} 
		else if (c == '=')
		{
			if (!inKey || hasEqual)
				return false;
			hasEqual = true;
			inKey = false;
		}
		else if (c == '&')
		{
			if (prev == '&' || !hasEqual)
				return false;
			inKey = true;
			hasEqual = false;
		}
		else
			return false;

		prev = c;
	}
	if (!hasEqual || prev == '&' || prev == '=')
		return false;

	return true;
}


bool Client::isValid()
{
	string path =_config->full_path;
	if (access(path.c_str(), F_OK) != 0)
	{
		_request.code = 404;
		return false;
	}
	
	if (access(path.c_str(), R_OK) != 0)
	{
		_request.code = 403;
		return false;
	}
	if (!isQueryStringValid())
	{
		_request.code = 400;
		return false;
	}
	return true;
}

char** Client::buildCgiEnv()
{
	vector<string> env;
	
	env.push_back("REQUEST_METHOD=" + _request.getMethod());
	env.push_back("SCRIPT_FILENAME=" + _config->full_path);
	env.push_back("REDIRECT_STATUS=200");

	if (_request.getMethod() == "GET")
		env.push_back("QUERY_STRING=" + _config->query_string);
	else if (_request.getMethod() == "POST")
	{
		stringstream ss;
		ss << _request.getBody().size();
		env.push_back("CONTENT_LENGTH=" + ss.str());
		
		map<string,string>::const_iterator it = _request.getHeaders().find("Content-Type");
		string contentType = "";
		if (it != _request.getHeaders().end())
			contentType = trim(it->second);
		env.push_back("CONTENT_TYPE=" + contentType);
	}

	char **envp = new char *[env.size() + 1];
	for (size_t i = 0; i < env.size(); ++i)
		envp[i] = strdup(env[i].c_str());
	envp[env.size()] = NULL;
	
	return envp;
}

int Client::buildHttpResponseFromCgiOutput(const std::string& cgiOutput)
{
	size_t pos = cgiOutput.find("\r\n\r\n");
	string headers, body;
	int statusCode = 200;
	string contentType = "text/html; charset=UTF-8";
	
	if (pos != string::npos)
	{
		headers = cgiOutput.substr(0, pos);
		body = cgiOutput.substr(pos + 4);

		size_t statusPos = headers.find("Status:");
		if (statusPos != string::npos)
			statusCode = atoi(headers.substr(statusPos + 7).c_str());
		
		size_t typePos = headers.find("Content-Type:");
		if(typePos != string::npos)
		{
			size_t end = headers.find("\r\n",typePos);
			contentType = headers.substr(typePos + 13, end - (typePos + 13));

			if (contentType.find("charset=") == std::string::npos)
				contentType += "; charset=UTF-8";
		}
	}
	else
		body = cgiOutput;
	_request.response.content_type = contentType;
	_request.response.body = body;
	return (statusCode);
}


int Client::handleCGI()
{
	if (!isValid())
		return (_request.code);
	
	char **envp = buildCgiEnv();

	std::string path =_config->full_path;
	std::string interpreter;
	
	if (path.rfind(".php") == path.size() - 4)
		interpreter = "/usr/bin/php-cgi";
	else if (path.rfind(".py") == path.size() - 3)
		interpreter = "/usr/bin/python3";
	else
		interpreter = "";

	char *argv[3];
	if (!interpreter.empty())
	{
		argv[0] = strdup(interpreter.c_str());
		argv[1] = strdup(path.c_str()); 
		argv[2] = NULL; 
	}
	else // binaire CGI exécutable directement
	{
		argv[0] = strdup(path.c_str());
		argv[1] = NULL;
	}
	
	int requestPipe[2]; //ce que le serveur envoie au script
	int responsePipe[2]; // ce que le script renvoie au serveur

	if (pipe(requestPipe) == -1 || pipe(responsePipe) == -1)
		return (500);

	pid_t pid = fork();
	if (pid < 0)
		return (500);
	
	if (pid == 0)
	{
		close(requestPipe[1]);
		close(responsePipe[0]);
		
		dup2(requestPipe[0], STDIN_FILENO);
		close(requestPipe[0]);

		dup2(responsePipe[1], STDOUT_FILENO);
		close(responsePipe[1]);
		
		if (!interpreter.empty())
		{
			if (execve(interpreter.c_str(), argv, envp) == -1)
				perror("execve");
			return (500);
		}
		else
		{
			if (execve(path.c_str(), argv, envp) == -1)
				perror("execve");
			return (500);
		}
	}
	
	close(requestPipe[0]);
	close(responsePipe[1]);

	if (_request.getMethod() == "POST")
	{
		for (vector<char>::const_iterator it = _request.getBody().begin(); it != _request.getBody().end(); it++)
			write(requestPipe[1], &(*it), 1);
	}
	close(requestPipe[1]);

	string cgiOutput;
	char buffer[4096];
	ssize_t n;
	while ((n = read(responsePipe[0], buffer, sizeof(buffer))) > 0)
		cgiOutput.append(buffer,n);
	close(responsePipe[0]);
	
	int status;
	waitpid(pid, &status, 0);

	for (size_t i = 0; argv[i]; ++i)
		free(argv[i]);

	for (size_t i = 0; envp[i]; ++i)
		free(envp[i]);
	delete[] envp;
	return (buildHttpResponseFromCgiOutput(cgiOutput));
}
