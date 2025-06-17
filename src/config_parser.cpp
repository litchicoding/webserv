#include "webserv.hpp"

int	parse_config_file(const std::string &config_file)
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
	if (tokenize_config_file(file, &tokenList) == ERROR) {
		file.close();
		return ERROR;
	}
	file.close();

	std::vector<t_tokenConfig>::iterator it = tokenList.begin();
	while (it != tokenList.end())
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
	return OK;
}