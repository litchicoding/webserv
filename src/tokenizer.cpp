#include "webserv.hpp"

static size_t	get_end_word_index(const std::string &line, size_t start)
{
	for (size_t i = start; i < line.size(); i++)
	{
		if (isspace(line[i]))
			return i - 1;
	}
	return std::string::npos;
}

size_t	skip_white_spaces(const std::string &line, size_t index)
{
	size_t	i = index;

	while (i < line.length())
	{
		if (!isspace(line[i]) || line[i] == '\n' || !line[i])
			return i;
		i++;
	}
	return i;
}

static t_tokenConfig	create_token(const std::string &data, t_tokenType tokenType)
{
	t_tokenConfig	token;
	token.data = data;
	token.type = tokenType;
	return token;
}

static int	tokenize_server_line(std::string &line, size_t start, std::vector<t_tokenConfig> *tokenList)
{
	tokenList->push_back(create_token("server", SERVER));
	size_t	i = skip_white_spaces(line, start + SERV_LEN);
	if (line[i] == '{' && line[i + 1] == '\0')
		tokenList->push_back(create_token("{", O_BRACE));
	else
		return parsing_error("tokenize_server_line", MISSING_ARG);
	return OK;
}

static int	tokenize_location_line(std::string &line, size_t start, std::vector<t_tokenConfig> *tokenList)
{
	std::string	arg;
	size_t		i = skip_white_spaces(line, start + LOC_LEN);
	
	start = i;
	while (line[i] && line[i] != '{' && !isspace(line[i]))
		i++;
	arg = line.substr(start, i - start);
	tokenList->push_back(create_token(arg, LOCATION));
	i = skip_white_spaces(line, i);
	if (line[i] == '{' && line[i + 1] == '\0')
		tokenList->push_back(create_token("{", O_BRACE));
	else
		return parsing_error("tokenize_location_line", MISSING_ARG);
	return OK;
}

static bool	is_directive_line(const std::string &line, size_t start)
{
	std::string	directive[] = { "listen", "server_name", "error_page", "client_max_body_size", "root", "autoindex", "index" };
	size_t		end = get_end_word_index(line, start) + 1;

	std::cout << line << std::endl;
	for (size_t i = 0; i < directive->size() + 1; ++i)
	{
		if (!line.compare(start, end - start, directive[i]))
			return true;
	}
	return false;
}

static int	tokenize_directive_line(std::string &line, size_t start, std::vector<t_tokenConfig> *tokenList)
{
	size_t	end = get_end_word_index(line, start);
	if (end == std::string::npos)
		return ERROR;
	tokenList->push_back(create_token(line.substr(start, end), IDENTIFIER));
	start = skip_white_spaces(line, end + 1);
	end = start;
	while (line[end] && line[end] != ';')
	{
		if (isspace(line[end])) {
			std::string	args = line.substr(start, end - start);
			tokenList->push_back(create_token(args, ARG));
			start = end;
			end = skip_white_spaces(line, end);
		}
		else if (line[end + 1] == ';') {
			end++;
			std::string	args = line.substr(start, end - start);
			tokenList->push_back(create_token(args, ARG));
			start = end;
			end = skip_white_spaces(line, end);
		}
		else
			end++;
	}
	if (line[end] && line[end] == ';')
		tokenList->push_back(create_token(";", SEMICOLON));
	else
		return parsing_error("tokenize_directive_line", MISSING_ARG);
	return OK;
}

int	tokenize_config_file(std::ifstream &file, std::vector<t_tokenConfig> *tokenList)
{
	std::string					line;
	size_t						pos = 0;
	int							status = OK;

	while (getline(file, line))
	{
		pos = skip_white_spaces(line, 0);
		if (!line.compare(pos, pos + SERV_LEN, "server"))
			status = tokenize_server_line(line, pos, tokenList);
		else if (!line.compare(pos, LOC_LEN, "location"))
			status = tokenize_location_line(line, pos, tokenList);
		else if (is_directive_line(line, pos))
			status = tokenize_directive_line(line, pos, tokenList);
		else if (line[pos] == '}')
			tokenList->push_back(create_token("}", C_BRACE));
		else if (line[pos] != '#' && line.empty() && line[pos] != '\0')
			return parsing_error("tokenize_config_file", INVALID_ARG);
		if (status != OK)
			return ERROR;
	}
	tokenList->push_back(create_token("EOF", END));
	return OK;
}

void print_token_type(std::vector<t_tokenConfig> *tokenList)
{
	std::vector<t_tokenConfig>::iterator it = tokenList->begin();
	while (it != tokenList->end())
	{
		std::cout << "data = " << it->data << std::endl;
		std::cout << "type = ";
		switch (it->type)
		{
			case SERVER:
				std::cout << "server" << std::endl;
				break ;
			case LOCATION:
				std::cout << "location" << std::endl;
				break ;
			case IDENTIFIER:
				std::cout << "identifier" << std::endl;
				break ;
			case O_BRACE:
				std::cout << "opening brace" << std::endl;
				break ;
			case C_BRACE:
				std::cout << "closing brace" << std::endl;
				break ;
			case SEMICOLON:
				std::cout << "semicolon" << std::endl;
				break ;
			case ARG:
				std::cout << "arg" << std::endl;
				break ;
			case END:
				std::cout << "end of file" << std::endl;
				break ;
		}
		std::cout << "------------------------------------" << std::endl;
		it++;
	}
}