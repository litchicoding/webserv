#include "webserv.hpp"

static int	parse_directive(vector<t_tokenConfig>::iterator &token, t_tokenType type, Server &server, const string &loc_path)
{
	string			directive_type = token->data;
	vector<string>	directive_arg;

	if (token->type != IDENTIFIER)
		return (parsing_error("directive type", MISSING_ARG));
	token++;
	while (token->type != SEMICOLON)
	{
		if (token->type == ARG && !token->data.empty()) {
			if (directive_type == "return" 
				&& ((token->data[0] == '"' && token->data[token->data.length() - 1] == '"')
				|| (token->data[0] == '\'' && token->data[token->data.length() - 1] == '\'')))
					token->data = token->data.substr(1, token->data.length() - 2);
			directive_arg.push_back(token->data);
		}
		else if (token->type == ARG && token->data.empty())
			return parsing_error("directive argument is empty", MISSING_ARG);
		token++;
	}
	if (token->type != SEMICOLON)
		return (parsing_error("directive argument", MISSING_ARG));
	if (type == SERVER)
		return (server.setOneDirective(directive_type, directive_arg, &server.getDirectives()));
	else if (type == LOCATION)
		return (server.setLocation(loc_path, directive_type, directive_arg));
	else
		return (ERROR);
	return (OK);
}

static int	parse_location_block(vector<t_tokenConfig>::iterator &token, Server &server)
{
	if (token->data.empty())
		return (parsing_error("location path argument is empty", INVALID_ARG));
		
	string	loc_path = token->data;
	token++;
	if (token->type != O_BRACE)
		return (parsing_error("opening brace", MISSING_ARG));
	while (token->type != C_BRACE)
	{
		if (token->type == IDENTIFIER && parse_directive(token, LOCATION, server, loc_path) != OK)
			return (ERROR);
		token++;
	}
	if (server.getLocations().find(loc_path) == server.getLocations().end())
		server.setLocation(loc_path, "", std::vector<std::string>());
	if (token->type != C_BRACE)
		return (parsing_error("closing brace", MISSING_ARG));
	return (OK);
}

static int	parse_server_block(vector<t_tokenConfig>::iterator &token, vector<Server> &serv_blocks)
{
	if (token->type != SERVER)
		return parsing_error("server block", MISSING_ARG);

	Server	newServer = Server();
	token++;
	if (token->type != O_BRACE)
		return parsing_error("opening brace", MISSING_ARG);
	token++;
	while (token->type != C_BRACE)
	{
		if (token->type == LOCATION && parse_location_block(token, newServer) != OK)
			return (ERROR);
		else if (token->type == IDENTIFIER && parse_directive(token, SERVER, newServer, "") != OK)
			return (ERROR);
		token++;
	}
	if (token->type != C_BRACE)
		return parsing_error("closing brace", MISSING_ARG);
	newServer.defaultConfiguration();
	serv_blocks.push_back(newServer);
	return (OK);
}

static bool isNameExtensionValid(const string &config_file, size_t len)
{
	size_t	delimiter;
	
	delimiter = config_file.find_last_of(".");
	if (delimiter != string::npos) {
		if (!config_file.compare(delimiter, len, ".conf") && config_file[len - 1] == 'f')
			return (true);
	}
	cout << RED "Error: wrong name extension for the configuration file" RESET << endl;
	return (false);
}

int	parse_config_file(string config_file, Listen &listenPorts)
{
	if (config_file.empty())
		config_file = DEFAULT_CONFIG_FILE;
	if (!isNameExtensionValid(config_file, config_file.length()))
		return ERROR;
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
		if (parse_server_block(it, serv_blocks) != OK) {
			cout << RED << "Can't continue: invalid format flagged in configuration file\n" << RESET;
			return ERROR;
		}
		it++;
	}
	if (serv_blocks.empty()) {
		serv_blocks.push_back(Server());
		serv_blocks.back().defaultConfiguration();
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
			if (msg.size() > 0)
				cout  << "missing an argument -> " << msg;
			else
				cout  << "missing an argument";
			break ;
		case INVALID_ARG:
			if (msg.size() > 0)
				cout << "invalid argument detected in -> \"" << msg << "\"";
			else
				cout << "invalid argument detected in ";
			break ;
		case INVALID_FORMAT:
			if (msg.size() > 0)
				cout << "invalid argument detected in -> " << msg;
			else
				cout << "invalid argument detected in ";
			break ;
	}
	cout << endl << RESET;
	return ERROR;
}
