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

# define SERV_LEN 6
# define LOC_LEN 8

# define MISSING_ARG 10
# define INVALID_ARG 11

typedef enum tokenType
{
	SERVER,
	LOCATION,
	IDENTIFIER,
	O_BRACE,
	C_BRACE,
	SEMICOLON,
	ARG,
	END
}					t_tokenType;

typedef struct	tokenConfig
{
	tokenType	type;
	std::string	data;	
}				t_tokenConfig;

int		parse_config_file(const std::string &config_file);
int		tokenize_config_file(std::ifstream &file, std::vector<t_tokenConfig> *tokenList);
int		parsing_error(const std::string &msg, int code_error);
size_t	skip_white_spaces(const std::string &line, size_t index);

#endif