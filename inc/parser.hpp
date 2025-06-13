#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>

/*
config = server_block

server_block = server + o_brace + directive + location_block + c_brace

location_block = location + args + o_brace + directive + c_brace

directive = identifier + args + colon

colon = ";"
o_brace = "{"
c_brace = "{"

args = a string

identifier = "listen", "server_name", "error_page", "client_max_body_size", "root", "autoindex", "index", "fastcgi_param", "fastcgi_pass"

server = "server"

*/

enum class TokenType
{
	SERVER,
	LOCATION,
	IDENTIFIER,
	O_BRACE,
	C_BRACE,
	SEMICOLON,
	ARG
};

struct	TokenConfigFile
{
	TokenType	type;
	std::string	data;	
};

#endif