#include "../inc/Client.hpp"

void	Client::handleGet() {
	if (access(URI.c_str(), F_OK) != 0)
		handleError(404);
	if (access(URI.c_str(), R_OK) != 0)
		handleError(403);
	if (HeadersCorrect("GET") != OK)
		handleError(???);
	std::ifstream	file(URI);
	if (!file.is_open())
		handleError(500);
	std::ostringstream	body;
	body << file.rdbuf();
	
	std::string MIME = getMIME(URI);
	
	this->response = "HTTP/1.1 200 OK\r\n";
	this->response += "Content-Type: " + MIME + "\r\n";
	this->response += "Content-Length: " + std::to_string(body.str().size()) + "\r\n";
	this->response += "Connection: keep-alive\r\n";
	this->response += "\r\n";
	this->response += body.str();
	return ;
}

void    Client::handlePost() {
    
}

void    Client::handleDelete() {
    if (access(URI.c_str(), F_OK) != 0)
        handleError(404);
    // comment faire sans unlink ?
    /*
    si reussie retour code 200 OK
    sinon retour code 500
    */
}