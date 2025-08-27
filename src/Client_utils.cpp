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
    for (size_t i = 0; i < URI.length(); i++)
    {
        char c = URI[i];
        if (!(c == 95 ||
            (c >= 45 && c <= 57) ||
            (c >= 64 && c <= 90) ||
            (c >= 97 && c <= 122)))
		{
			if (c == '%')
			{
				if (i+2 >= URI.size() || !isxdigit(URI[i+1]) || !isxdigit(URI[i+2]))
					return true;
				i += 2;
				continue;
			}
            return true;
		}
    }
    return false;
}

string Client::urlDecode(const string &str)
{
    string result;
    size_t i = 0;

    while (i < str.length()) {
        if (str[i] == '%' && i + 2 < str.length() &&
            isxdigit(str[i+1]) && isxdigit(str[i+2])) {
            string hex = str.substr(i + 1, 2);
            char decodedChar = static_cast<char>(strtol(hex.c_str(), 0, 16));
            result += decodedChar;
            i += 3;
        } else if (str[i] == '+') {
            result += ' ';
            i++;
        } else {
            result += str[i++];
        }
    }

    return result;
}
