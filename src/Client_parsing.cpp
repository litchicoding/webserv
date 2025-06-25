#include "../inc/Client.hpp"

std::string	Client::collect_request() {
	std::string		newRequest;
	const size_t	buffer_size = 1024;
	char			buffer[buffer_size];

	while (true) {
		std::memset(buffer, 0, buffer_size);
		ssize_t bytes_read = recv(this->getSocketFd(), buffer, buffer_size - 1, 0);
		if (bytes_read < 0) {
			break ;
		}
		else if (bytes_read == 0)
			break ;
		else
			newRequest.append(buffer, bytes_read);
	}
	return newRequest;
}

void	Client::handleMethodLine(std::string& line)
{
	std::istringstream  iss(line);
	if (!(iss >> this->_method >> this->_URI >> this->_version))
		return(handleError(400));
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return(handleError(405));
	if (_URI.empty() || _URI[0] != '/')
		return(handleError(400));
	if (_URI.find("..") != std::string::npos)
		return(handleError(403));
	if (URI_Not_Printable(_URI))
		return(handleError(400));
	if (_URI.size() > 2048)
		return(handleError(414));
	if (_version != "HTTP/1.1")
		return(handleError(505));
	// else
	// {
	//     std::string tmp = _URI;
	//     this->_URI = root + tmp;
	// }

	// TODO : Ajouter protection contre les chemins hors root
	
	return ;
}

void	Client::handleHeaders(std::string& line)
{
	size_t delimiterPos = line.find(':');
	if (delimiterPos == std::string::npos)
		return(handleError(400));
	std::string key = line.substr(0, delimiterPos);
	std::string value = line.substr(delimiterPos + 1);
	if (key.empty())
		return(handleError(400));
	this->_headersMap[key] = value;

	// if (HeadersCorrect("GET") != OK)
	// 	handleError(400);
}


void    Client::handleBody(std::string& line)
{
	this->_body.append(line + "\n");
}
