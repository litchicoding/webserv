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
		contentType = "image/gif";
	else if (URI.find(".otf") != std::string::npos)
		contentType = "font/otf";
	else if (URI.find(".svg") != std::string::npos)
		contentType = "image/svg+xml";
	else if (URI.find(".css") != std::string::npos)
		contentType = "text/css";
	else if (URI.find(".ico") != std::string::npos)
		contentType = "image/x-icon";
	else
		contentType = "text/plain";
	return contentType;
}

bool	Client::URI_Not_Printable(std::string& URI)
{
	static const std::string allowed =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		" ._~:/?#[]@!$&'()*+,;=%";

	for (size_t i = 0; i < URI.length(); i++)
	{
		if (allowed.find(URI[i]) == std::string::npos)
		    return true;
	}
	return false;
}
