#include "Utils.hpp"

std::string getMIME(std::string fd)
{
    std::string contentType;
	if (fd.find(".html") != std::string::npos)
		contentType = "text/html";
	else if (fd.find(".jpg") != std::string::npos)
		contentType = "image/jpeg";
	else if (fd.find(".png") != std::string::npos)
		contentType = "icon/png";
	else if (fd.find(".ttf") != std::string::npos)
		contentType = "font/ttf";
	else if (fd.find(".js") != std::string::npos)
		contentType = "tet/javascript";
	else if (fd.find(".css") != std::string::npos)
		contentType = "text/css";
	else
		contentType = "text/txt";
    return contentType;
}