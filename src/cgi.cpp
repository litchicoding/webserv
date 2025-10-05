#include "Client.hpp"

bool Client::isCgi()
{
	if (!_config)
		return false;
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
	size_t sepLen = 4;
	if (pos == std::string::npos)
	{
		pos = cgiOutput.find("\n\n");
		sepLen = 2;
	}
	
	string headers, body;
	int statusCode = 200;
	string contentType = "text/html; charset=UTF-8";
	
	if (pos != string::npos)
	{
		headers = cgiOutput.substr(0, pos);
		body = cgiOutput.substr(pos + sepLen);

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

void	Client::processCGI(int fd)
{
	if (!_cgi.is_running)
		return;
	// POST
	if (fd == _cgi.stdin_fd)
	{
		for (vector<char>::const_iterator it = _request.getBody().begin(); it != _request.getBody().end(); it++)
			write(fd, &(*it), 1);
		close (fd);
	}
	//lecture du cgi
	char buf[4096];
	ssize_t n;
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		_cgi.buffer.append(buf, n);
	
	if (n == 0) //fin du flux
	{
		close(fd);
		_cgi.is_running = false;
		int status_code = buildHttpResponseFromCgiOutput(_cgi.buffer);
		buildResponse(status_code);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);

		// for (int i = 0; _cgi.envp && _cgi.envp[i]; ++i)
		// 	free(_cgi.envp[i]);
		// delete[] _cgi.envp;

		// for (int i = 0; i < 3 && _cgi.argv[i]; ++i)
		// 	free(_cgi.argv[i]);
	}
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
		
		if (dup2(requestPipe[0], STDIN_FILENO) == -1)
			exit (1);
		close(requestPipe[0]);

		if (dup2(responsePipe[1], STDOUT_FILENO) == -1)
			exit (1);
		close(responsePipe[1]);

		if (!interpreter.empty())
		{
			execve(interpreter.c_str(), argv, envp); 
			perror("execve");
			exit (1);
		}
		else
		{
			execve(path.c_str(), argv, envp);
			perror("execve");
			exit (1);
		}
	}
	
	close(requestPipe[0]);
	close(responsePipe[1]);

	for (int i = 0; envp && envp[i]; ++i)
			free(envp[i]);
	delete[] envp;

	for (int i = 0; i < 3 && argv[i]; ++i)
		free(argv[i]);

	_cgi.pid = pid;
	_cgi.stdin_fd = requestPipe[1];
	_cgi.stdout_fd = responsePipe[0];
	_cgi.is_running = true;
	_cgi.buffer.clear();

	epoll_event ev;
	if (_request.getMethod() == "POST")
	{
		ev.events = EPOLLOUT;
		ev.data.fd = _cgi.stdin_fd;
		epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _cgi.stdin_fd, &ev);
		_listen->_cgi_fds[_cgi.stdin_fd] = this;
	}
	ev.events = EPOLLIN;
	ev.data.fd = _cgi.stdout_fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _cgi.stdout_fd, &ev);
	_listen->_cgi_fds[_cgi.stdout_fd] = this;
	return (0);
}
