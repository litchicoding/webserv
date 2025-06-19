#include "Utils.hpp"

std::string getMIME(std::string fd)
{
    std::string contentType;
	if (fd.find(".html") != std::string::npos)
		contentType = "text/html";
	else if (fd.find(".jpg") != std::string::npos || fd.find(".jpeg") != std::string::npos)
		contentType = "image/jpeg";
	else if (fd.find(".png") != std::string::npos)
		contentType = "image/png";
	else if (fd.find(".ttf") != std::string::npos)
		contentType = "font/ttf";
	else if (fd.find(".js") != std::string::npos)
		contentType = "text/javascript";
	else if (fd.find(".gif") != std::string::npos)
        return "image/gif";
	else if (fd.find(".otf") != std::string::npos)
        return "font/otf";
    else if (fd.find(".svg") != std::string::npos)
        return "image/svg+xml";
	else if (fd.find(".css") != std::string::npos)
		contentType = "text/css";
    else if (fd.find(".ico") != std::string::npos)
        return "image/x-icon";
	else
		contentType = "text/plain";
    return contentType;
}