#include "webserv.hpp"

static int	parse_directive(std::vector<t_tokenConfig>::iterator &token, t_tokenType type, Server *server, const std::string &loc_path)
{
	std::string					directive_type = token->data;
	std::vector<std::string>	directive_arg;

	token++;
	while (token->type != SEMICOLON)
	{
		if (token->type == ARG && !token->data.empty())
			directive_arg.push_back(token->data);
		else
			return parsing_error("parse_directive", MISSING_ARG);
		token++;
	}
	if (type == SERVER)
		server->setDirectives(directive_type, directive_arg);
	else if (type == LOCATION)
		server->setLocation(loc_path, directive_type, directive_arg);
	return OK;
}

static int	parse_location_block(std::vector<t_tokenConfig>::iterator &token, Server *server)
{
	if (token->data.empty())
		return ERROR;
	std::string					loc_path = token->data;
	
	token++;
	if (token->type != O_BRACE)
		return parsing_error("parse_location_block", MISSING_ARG);
	while (token->type != C_BRACE)
	{
		if (token->type == IDENTIFIER && parse_directive(token, LOCATION, server, loc_path) != OK)
			return ERROR;
		token++;
	}
	return OK;
}

static int	parse_server_block(std::vector<t_tokenConfig>::iterator &token, std::vector<Server*> &serv_group)
{
	if (token->type != SERVER)
		return parsing_error("parse_server_block", MISSING_ARG);

	Server	*newServer = new Server();
	token++;
	if (token->type != O_BRACE)
		return parsing_error("parse_server_block", MISSING_ARG);
	while (token->type != C_BRACE)
	{
		if (token->type == LOCATION && parse_location_block(token, newServer) != OK)
			return ERROR;
		else if (token->type == IDENTIFIER && parse_directive(token, SERVER, newServer, "") != OK)
			return ERROR;
		token++;
	}
	serv_group.push_back(newServer);
	return OK;
}

int	parse_config_file(const std::string &config_file, std::vector<Server*> &serv_group)
{
	// open file
	std::ifstream	file;

	file.open(config_file.c_str());
	if (!file.is_open() || file.fail()) {
		std::cout << RED <<  "Error: can't open file" << RESET << std::endl;
		return ERROR;
	}

	// read file and store token
	std::vector<t_tokenConfig>	tokenList;
	if (tokenize_config_file(file, &tokenList) != OK) {
		file.close();
		return ERROR;
	}
	file.close();
	
	// just for debug -- print every token node
	print_token_type(&tokenList);

	// parse every token node and create new Server object accordingly, store them in serv_group
	std::vector<t_tokenConfig>::iterator it = tokenList.begin();
	while (it->type != END)
	{
		if (parse_server_block(it, serv_group) != OK)
			return ERROR;
		it++;
	}

	for (std::vector<Server*>::iterator it = serv_group.begin(); it != serv_group.end(); it++)
		(*it)->print_server_class();

	return OK;
}

int	parsing_error(const std::string &msg, int code_error)
{
	std::cout << RED << "Error: ";
	switch (code_error)
	{
		case MISSING_ARG:
			std::cout << msg << " - missing an argument" << std::endl;
			break ;
		case INVALID_ARG:
			std::cout << msg << " - invalid argument detected" << std::endl;
			break ;
	}
	std::cout << RESET;
	return ERROR;
}

