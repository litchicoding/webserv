#include "../inc/Client.hpp"

struct stat st;

void	Client::handleGet() {
	if (access(URI.c_str(), F_OK) != 0)
	{
		std::cout << RED "ACCESS OUUUUU" RESET << std::endl;
		return(handleError(404));
	}
	// if (stat(URI.c_str(), &st) != 0)
	// 	return(handleError(500));
	// if (!S_ISREG(st.st_mode))
	// 	return(handleError(404));
	if (access(URI.c_str(), R_OK) != 0)
		return(handleError(403));
	// if (HeadersCorrect("GET") != OK)
	// 	handleError(400);
	std::ifstream	file(URI.c_str());
	if (!file.is_open())
		return (handleError(500));
	std::ostringstream	body;
	body << file.rdbuf();
	
	std::string MIME = getMIME(URI);
	
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
	if (access(URI.c_str(), F_OK | W_OK) != 0)
		return(handleError(404)); // pas sur du code ici !
	std::ofstream	outfile(URI.c_str());
	if (!outfile.is_open())
		return(handleError(500));
	
	outfile.write(body.c_str(), body.size());
	outfile.close();
	this->response = "HTTP/1.1 201 Created\r\n";
	this->response += "Content-Length: 0\r\n";
	this->response += "Connection: keep-alive\r\n\r\n";
	return ;
}

void	Client::handleDelete() {
	if (access(URI.c_str(), F_OK) != 0)
		return(handleError(404));
	if (std::remove(URI.c_str()) != 0)
		return(handleError(500));
		// headers
	else
	{
		std::cout << RED << "DELETE : Reussie !" << RESET << std::endl;
		
		// this->response = "HTTP/1.1 200 OK\r\n";
		// this->response += "Content-Type: " + MIME + "\r\n";
		// this->response += "Content-Length: " + std::to_string(body.str().size()) + "\r\n";
		// this->response += "Connection: keep-alive\r\n";
		// this->response += "\r\n";
		// this->response += body.str();
		return ;
	}
}