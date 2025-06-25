#include "../inc/Client.hpp"

struct stat st;

int	Client::HeadersCorrect(std::string method)
{
	std::map<std::string, std::string>::iterator it;
	std::map<std::string, std::string>::iterator it2;
	it = _headersMap.find("Host");
	if (it == _headersMap.end())
		return ERROR;
	if (method == "POST")
	{
		it = _headersMap.find("Content-Length");
		it2 = _headersMap.find("Transfert-Encoding");
		if (it == _headersMap.end() && it2 == _headersMap.end())
			return ERROR;
		if (it != _headersMap.end() && it2 != _headersMap.end())
			return ERROR;
		it = _headersMap.find("Content-Type");
		if (it == _headersMap.end())
			return ERROR;
	}
	return OK;
}

void	Client::handleGet() {
	if (access(_URI.c_str(), F_OK) != 0)
	{
		std::cout << RED "ACCESS OUUUUU" RESET << std::endl;
		return(handleError(404));
	}
	if (stat(_URI.c_str(), &st) != 0)
		return(handleError(500));
	if (!S_ISREG(st.st_mode))
		return(handleError(404));
	if (access(_URI.c_str(), R_OK) != 0)
		return(handleError(403));
	if (HeadersCorrect("GET") != OK)
		handleError(400);

	std::ifstream	file(_URI.c_str());
	if (!file.is_open())
		return (handleError(500));
	std::ostringstream	body;
	body << file.rdbuf();
	
	std::string MIME = getMIME(_URI);
	
	std::cout << RED << "GET : Reussie !" << RESET << std::endl;
	
	// this->response = "HTTP/1.1 200 OK\r\n";
	// this->response += "Content-Type: " + MIME + "\r\n";
	// this->response += "Content-Length: " + std::to_string(body.str().size()) + "\r\n";
	// this->response += "Connection: keep-alive\r\n";
	// this->response += "\r\n";
	// this->response += body.str();
	return ;
}

void	Client::handlePost() {
	if (access(_URI.c_str(), F_OK | W_OK) != 0)
		return(handleError(404)); // pas sur du code ici !
	std::ofstream	outfile(_URI.c_str());
	if (!outfile.is_open())
		return(handleError(500));
	
	outfile.write(_body.c_str(), _body.size());
	outfile.close();
	this->_response = "HTTP/1.1 201 Created\r\n";
	this->_response += "Content-Length: 0\r\n";
	this->_response += "Connection: keep-alive\r\n\r\n";
	return ;
}

void	Client::handleDelete() {
	if (access(_URI.c_str(), F_OK) != 0)
		return(handleError(404));
	if (std::remove(_URI.c_str()) != 0)
		return(handleError(500));
		// headers
	else
	{
		std::cout << RED << "DELETE : Reussie !" << RESET << std::endl;
		
		// this->_response = "HTTP/1.1 200 OK\r\n";
		// this->_response += "Content-Type: " + MIME + "\r\n";
		// this->_response += "Content-Length: " + std::to_string(body.str().size()) + "\r\n";
		// this->_response += "Connection: keep-alive\r\n";
		// this->_response += "\r\n";
		// this->_response += body.str();
		return ;
	}
}