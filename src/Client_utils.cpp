#include "../inc/Client.hpp"

std::string Client::getMIME(std::string& URI)
{
    std::string contentType;
	if (URI.find(".html") != std::string::npos)
		contentType = "text/html";
	else if (URI.find(".jpg") != std::string::npos || URI.find(".jpeg") != std::string::npos)
		contentType = "image/jpeg";
	else if (URI.find(".png") != std::string::npos)
		contentType = "image/png";
	else if (URI.find(".ttf") != std::string::npos)
		contentType = "font/ttf";
	else if (URI.find(".js") != std::string::npos)
		contentType = "text/javascript";
	else if (URI.find(".gif") != std::string::npos)
        return "image/gif";
	else if (URI.find(".otf") != std::string::npos)
        return "font/otf";
    else if (URI.find(".svg") != std::string::npos)
        return "image/svg+xml";
	else if (URI.find(".css") != std::string::npos)
		contentType = "text/css";
    else if (URI.find(".ico") != std::string::npos)
        return "image/x-icon";
	else
		contentType = "text/plain";
    return contentType;
}