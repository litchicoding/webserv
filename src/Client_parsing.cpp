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
	iss >> this->method >> this->URI >> this->version;
    
	if (method != "GET" && method != "POST" && method != "DELETE")
        return(handleError(405));
	if (URI.empty() || URI[0] != '/')
        return(handleError(400));
    // else
    // {
        path + root + URI
    //     std::string tmp = URI;
    //     this->URI = root + tmp;
    // }
	if (URI.find("..") != std::string::npos)
        return(handleError(403));
	if (version != "HTTP/1.1")
        return(handleError(505));
	// rajouter si fichier sors du root du config ?? 403
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
    this->headersMap[key] = value;
}

void    Client::handleBody(std::string& line)
{
    this->body.append(line + "\n");
}
