#include "webserv.hpp"

static int	parse_directive(vector<t_tokenConfig>::iterator &token, t_tokenType type, Server &server, const string &loc_path)
{
	string			directive_type = token->data;
	vector<string>	directive_arg;

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
		server.setOneDirective(directive_type, directive_arg, &server.getDirectives());
	else if (type == LOCATION)
		server.setLocation(loc_path, directive_type, directive_arg);
	return OK;
}

static int	parse_location_block(vector<t_tokenConfig>::iterator &token, Server &server)
{
	if (token->data.empty())
		return ERROR;
		
	string	loc_path = token->data;
	token++;
	if (token->type != O_BRACE)
		return parsing_error("parse_location_block", MISSING_ARG);
	while (token->type != C_BRACE)
	{
		if (token->type == IDENTIFIER && parse_directive(token, LOCATION, server, loc_path) != OK)
			return ERROR;
		token++;
	}
	if (server.getLocations().find(loc_path) == server.getLocations().end())
		server.setLocation(loc_path, "", std::vector<std::string>());
	return OK;
}

static int	parse_server_block(vector<t_tokenConfig>::iterator &token, vector<Server> &serv_blocks)
{
	if (token->type != SERVER)
		return parsing_error("parse_server_block", MISSING_ARG);

	Server	newServer = Server();
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
	newServer.defaultConfiguration();
	serv_blocks.push_back(newServer);
	return OK;
}

int	parse_config_file(string config_file, Listen &listenPorts)
{
	// TO DO -> check error in file name (extension etc) !!!
	if (config_file.empty())
		config_file = DEFAULT_CONFIG_FILE;

	// open file
	ifstream	file;

	file.open(config_file.c_str());
	if (!file.is_open() || file.fail()) {
		cout << RED <<  "Error: can't open file" << RESET << endl;
		return ERROR;
	}

	// read file and store token
	vector<t_tokenConfig>	tokenList;
	if (tokenize_config_file(file, &tokenList) != OK) {
		file.close();
		return ERROR;
	}
	file.close();
	
	// just for debug -- print every token node
	// print_token_type(&tokenList);

	// parse every token node and create new Server object accordingly, store them in serv_blocks
	vector<Server>						serv_blocks;
	vector<t_tokenConfig>::iterator	it = tokenList.begin();
	while (it->type != END)
	{
		if (parse_server_block(it, serv_blocks) != OK)
			return ERROR;
		it++;
	}
	listenPorts.setServerBlocks(serv_blocks);
	return OK;
}

int	parsing_error(const string &msg, int code_error)
{
	cout << RED << "Error: ";
	switch (code_error)
	{
		case MISSING_ARG:
			cout << msg << " - missing an argument" << endl;
			break ;
		case INVALID_ARG:
			cout << msg << " - invalid argument detected" << endl;
			break ;
	}
	cout << RESET;
	return ERROR;
}
