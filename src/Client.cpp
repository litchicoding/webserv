#include "Client.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Client::Client(int listen_fd, int epoll_fd) : _listen_fd(listen_fd), _server_config(NULL), _config(NULL)
{
	cout << GREEN << "***   Client Connection   ***" << RESET << endl;
	socklen_t	client_addr_len = sizeof(_client_addr);
	_client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&_client_addr), &client_addr_len);
	if (_client_fd == INVALID) {
		perror("accept");
		return ;
	}
	add_fd_to_epoll(epoll_fd, _client_fd);
	_server_config = NULL;
	_config = NULL;
	_chunked = false;
	_bodyCompleted = true;
	_content_len_target = INVALID;
	_request_len = 0;
	_response_len = 0;
}

Client::~Client()
{
	cout << GREEN << "***   Client Deconstruction   ***" << RESET << endl;
}

/**************************************************************************************************/
/* Parsing ****************************************************************************************/

bool	Client::isChunked()
{
	if (_bodyCompleted == false && _method == "POST")
		return (true);
	return (false);
}

int	Client::requestAnalysis()
{
	// Parse the request and start filling datas in client class
	if (parseRawRequest() != OK || request_well_formed_optimized() != OK
		|| _chunked == true || _bodyCompleted == false)
		return (ERROR);
	start();
	return (OK);
}

void	Client::start()
{
	if (_method == "GET")
		handleGet();
	if (_method == "POST")
		handlePost();
	if (_method == "DELETE")
		handleDelete();
}

int	Client::parseRawRequest() {
	istringstream  iss(this->_request);
	string line;
	bool isFirstLine = true;
	bool headers = true;

	while (getline(iss, line))
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1, 1);
		if (isFirstLine)
		{
			if (handleMethodLine(line) != OK)
				return ERROR;
			isFirstLine = false;
		}
		else if (line.empty())
			headers = false;
		else if (headers)
		{
			if (handleHeaders(line) != OK)
				return ERROR;
		}
		else
			handleBody(line);
		
	}
	return OK;
}


/**************************************************************************************************/
/* Member Fucntions *******************************************************************************/

void	Client::sendResponse(int client_fd)
{
	write(client_fd, _response.c_str(), _response_len);
	cout << CYAN << "   - RESPONSE TO REQUEST [socket:" << _client_fd << "] : " << RESET;
	size_t pos = _response.find("\n");
	cout << _response.substr(0, pos) << endl << endl;
}

void	Client::removeRequest()
{
	_server_config = NULL;
	_config = NULL;
	_chunked = false;
	_bodyCompleted = true;
	_method.clear();
	_URI.clear();
	_request.clear();
	_headersMap.clear();
	_body.clear();
	_root.clear();
	_response.clear();
	_request_len = 0;
	_content_len_target = INVALID;
	_response_len = INVALID;
}

void	Client::setRequest(const string &request, const int &len)
{
	if (_chunked == true) {
		_body.append(request, len);
		return ;
	}
	_request = request;
	_request_len = len;
}

void	Client::setServerConfig(Server *server_config) { _server_config = server_config; }

void	Client::setConfig()
{
	if (_server_config == NULL)
		return ;
	_config = _server_config->searchLocationMatch(_URI);
	if (_config == NULL) {
		cout << RED << "Error: setLocationMatch(): no match found with URI(" << _URI;
		cout << ")" << RESET << endl;
		return ;
	}
}

/**************************************************************************************************/
/* Getters ****************************************************************************************/

int	Client::getClientFd() { return _client_fd; }

Server*	Client::getServerConfig() const { return _server_config; }

string Client::getMethod() { return this->_method; }

string Client::getURI() { return this->_URI; }

string Client::getVersion() { return this->_version; }

string Client::getRequest() { return this->_request; }

string	Client::getResponse() const { return _response; }

size_t	Client::getResponseLen() const { return _response_len; }

int	Client::getListenFd() const { return _listen_fd; }