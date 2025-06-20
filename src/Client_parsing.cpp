#include "../inc/Client.hpp"

std::string	Client::collect_request() {
	std::string		newRequest;
	const size_t	buffer_size = 1024;
	char			buffer[buffer_size];

	while (true) {
		std::memset(buffer, 0, buffer_size);
		ssize_t bytes_read = recv(this->getSocketFd(), buffer, buffer_size - 1, 0);
		if (bytes_read < 0) {
			throw std::runtime_error("Error: ..."); // ERROR numm ???
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
	iss >> this->method >> this->URI >> this->version;
    
	if (method != "GET" && method != "POST" && method != "DELETE")
    handleError(405);
	if (URI.empty() || URI[0] != '/')
    handleError(400);
	if (URI.find("..") != std::string::npos)
    handleError(403);
	if (version != "HTTP/1.1")
    handleError(505);
	// rajouter si fichier sors du root du config ?? 403
}

void	Client::handleHeaders(std::string& line)
{
    size_t delimiterPos = line.find(':');
    if (delimiterPos == std::string::npos)
        handleError(400);
    std::string key = line.substr(0, delimiterPos);
    std::string value = line.substr(delimiterPos + 1);
    if (key.empty())
        handleError(400);
    this->headersMap[key] = value;
}

void    Client::handleBody(std::string& line)
{
    this->body.append(line + "\n");
}
