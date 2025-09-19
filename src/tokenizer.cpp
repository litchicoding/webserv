#include "webserv.hpp"

static size_t	get_end_word_index(const string &line, size_t start)
{
	for (size_t i = start; i < line.size(); i++)
	{
		if (isspace(line[i]))
			return i - 1;
	}
	return string::npos;
}

size_t	skip_white_spaces(const string &line, size_t index)
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

static t_tokenConfig	create_token(const string &data, t_tokenType tokenType)
{
	t_tokenConfig	token;
	token.data = data;
	token.type = tokenType;
	return token;
}

static int	tokenize_server_line(string &line, size_t start, vector<t_tokenConfig> *tokenList)
{
	tokenList->push_back(create_token("server", SERVER));
	size_t	i = skip_white_spaces(line, start + SERV_LEN);

	if (line[i] == '{' && line[i + 1] == '\0')
		tokenList->push_back(create_token("{", O_BRACE));
	else if (line[i] == '{' && line[i + 1] != '\0')
		return parsing_error(line, INVALID_ARG);
	else
		return parsing_error("opening brace", MISSING_ARG);
	return OK;
}

static int	tokenize_location_line(string &line, size_t start, vector<t_tokenConfig> *tokenList)
{
	string	arg;
	size_t		i = skip_white_spaces(line, start + LOC_LEN);
	
	start = i;
	while (line[i] && line[i] != '{' && !isspace(line[i]))
		i++;
	arg = line.substr(start, i - start);
	tokenList->push_back(create_token(arg, LOCATION));
	i = skip_white_spaces(line, i);
	if (line[i] == '{' && line[i + 1] == '\0')
		tokenList->push_back(create_token("{", O_BRACE));
	else if (line[i] == '{' && line[i + 1] != '\0')
		return parsing_error(line, INVALID_ARG);
	else
		return parsing_error("opening brace", MISSING_ARG);
	return OK;
}

static bool	is_directive_line(const string &line, size_t start)
{
	string	directive[8] = { "listen", "error_page", "client_max_body_size", "root", "autoindex", "index", "allow_methods", "return" };
	size_t	end = get_end_word_index(line, start) + 1;

	for (size_t i = 0; i < directive->size() + 2; i++)
	{
		if (!line.compare(start, end - start, directive[i]))
			return true;
	}
	return false;
}

static int	tokenize_directive_line(string &line, size_t start, vector<t_tokenConfig> *tokenList)
{
	size_t	end = get_end_word_index(line, start) + 1;
	if (end == string::npos)
		return ERROR;

	tokenList->push_back(create_token(line.substr(start, end - start), IDENTIFIER));
	int	special_case = NO;
	if (tokenList->back().data == "return")
		special_case = YES;
	start = skip_white_spaces(line, end);
	end = start;
	while (line[end] && line[end] != ';')
	{
		if (isspace(line[end]) && special_case == YES) {
			string	args = line.substr(start, end - start);
			tokenList->push_back(create_token(args, ARG));
			start = end + 1;
			end = skip_white_spaces(line, end);
			special_case = DONE;
		}
		if (isspace(line[end]) && !special_case) {
			string	args = line.substr(start, end - start);
			tokenList->push_back(create_token(args, ARG));
			start = end + 1;
			end = skip_white_spaces(line, end);
		}
		else if (line[end + 1] == ';') {
			end++;
			string	args = line.substr(start, end - start);
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
		return parsing_error("semicolon", MISSING_ARG);
	return OK;
}

int	tokenize_config_file(ifstream &file, vector<t_tokenConfig> *tokenList)
{
	string					line;
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
		else if (line[pos] != '#' && !line.empty() && line[pos] != '\0')
			return parsing_error(line, INVALID_ARG);
		if (status != OK)
			return ERROR;
	}
	tokenList->push_back(create_token("EOF", END));
	return OK;
}

void print_token_type(vector<t_tokenConfig> *tokenList)
{
	vector<t_tokenConfig>::iterator it = tokenList->begin();
	while (it != tokenList->end())
	{
		cout << "data = " << it->data << "$" << endl;
		cout << "type = ";
		switch (it->type)
		{
			case SERVER:
				cout << "server" << endl;
				break ;
			case LOCATION:
				cout << "location" << endl;
				break ;
			case IDENTIFIER:
				cout << "identifier" << endl;
				break ;
			case O_BRACE:
				cout << "opening brace" << endl;
				break ;
			case C_BRACE:
				cout << "closing brace" << endl;
				break ;
			case SEMICOLON:
				cout << "semicolon" << endl;
				break ;
			case ARG:
				cout << "arg" << endl;
				break ;
			case END:
				cout << "end of file" << endl;
				break ;
		}
		cout << "------------------------------------" << endl;
		it++;
	}
}