#include "../inc/Client.hpp"

void	Client::sendRedirect(string URI)
{
	ostringstream response;

	response << "HTTP/1.1 301 Moved Permanently" << "\r\n";
	response << "Location: " << URI << "\r\n";
	response << "Connection: close\r\n";
	response << "Content-Length: 0\r\n";
	response << "\r\n";

	_response = response.str();
	_response_len = _response.size();
}

void	Client::handleError(int code)
{
	string	message, filePath;

	switch (code) {
		case 400:
			cout << RED << "400 : Reussie !" << endl;
			message = "400 Bad Request";
			filePath = "www/error_pages/400.html";
			break;
		case 403:
			cout << RED << "403 : Reussie !" << endl;
  			message = "403 Forbidden";
			filePath = "www/error_pages/403.html";
			break;
		case 404:
			cout << RED << "404 : Reussie !" << endl;
			message = "404 Not Found";
			filePath = "www/error_pages/404.html";			
			break;
		case 405:
			cout << RED << "405 : Reussie !" << endl;
			message = "405 Method Not Allowed";
			filePath = "www/error_pages/405.html";			
			break;
		case 500:
			cout << RED << "500 : Reussie !" << endl;
			message = "500 Internal Server Error";
			filePath = "www/error_pages/500.html";			
			break;
		case 505:
			cout << RED << "505 : Reussie !" << endl;
			message = "505 HTTP Version Not Supported";
			filePath = "www/error_pages/505.html";			
			break;
	}

	ifstream 		file(filePath.c_str());
	if (!file.is_open()) {
		cerr << "handleError(): could not open error page file: " << filePath << endl;
		
		ostringstream body;
		body << "<html><body><h1>" << message << "</h1></body></html>";

		ostringstream response;
		response << "HTTP/1.1 " << message << "\r\n";
		response << "Content-Type: text/html\r\n";
		response << "Content-Length: " << body.str().size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n";
		response << body.str();

		_response = response.str();
		_response_len = _response.size();
		return ;
	}

	ostringstream	body;
	body << file.rdbuf();
	file.close();

	ostringstream response;
	response << "HTTP/1.1 " << message << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body.str().size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body.str();

	_response = response.str();
	_response_len = _response.size();

	cout << RED << message << " sent to client." << endl;
}
