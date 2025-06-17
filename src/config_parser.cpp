#include "parser.hpp"

int	parse_config_file(const std::string &config_file)
{
	// open file
	std::ifstream	file;
	std::string		line;

	file.open(config_file.c_str());
	if (file.is_open() || file.fail()) {
		std::cout << RED <<  "Error: can't open file" << RESET << std::endl;
		return ERROR;
	}
	
}