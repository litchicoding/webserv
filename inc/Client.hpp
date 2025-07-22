#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

class	Server;

typedef struct s_directives t_directives;
struct stat st;

class	Client
{
private:
	/* Client ID **********************************************************************************/
	int									_client_fd;
	int									_listen_fd;
	struct sockaddr_in					_client_addr;
	Server								*_server_config;
	t_directives						*_config;
	/* Request ************************************************************************************/
	std::string							_method;
	std::string							_URI;
	std::string							_version;
	std::string							_request;
	std::map<std::string, std::string>	_headersMap;
	std::string							_body;
	std::string							_response;
	std::string							_root;
	int									_request_len;
	size_t								_response_len;

	/* Parsing ************************************************************************************/
	void								handleMethodLine(std::string& line);
	void								handleHeaders(std::string& line);
	void								handleBody(std::string& line);
	std::string							collect_request(); // a enlever je crois.
	std::string							getMIME(std::string& URI);
	bool								URI_Not_Printable(std::string& URI);
	int									HeadersCorrect(std::string method);
	
	void								handlePost();
	void								handleError(int code);
	bool								URI_has_slash_in_end();
	
	/* HandleDelete Function ****************************************************************************/

	void								handleDelete();
	void    							isFileDelete();
	void  								isDirectoryDelete();
	int									delete_all_folder_content(std::string URI);

	/* HandleGet Function ****************************************************************************/
	void								handleGet();
	void								handleFileRequest();
	void								handleDirectoryRequest();
	std::string							findIndexFile();
										// generateDirectoryListing();

public:
	Client(int listen_fd, int epoll_fd);
	// Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	/* Member Function ****************************************************************************/
	void								start();
	void								parseRawRequest();
	void								buildResponse();
	void    							request_well_formed_optimized();

	/* Setters ************************************************************************************/
	void								setRequest(const std::string &request, const int &len);
	void								setServerConfig(Server *server_config);
	void								setConfig();

	/* Getters ************************************************************************************/
	int									getClientFd();
	std::string							getMethod();
	std::string							getURI();
	std::string							getVersion();
	std::string							getRequest();
	Server*								getServerConfig() const;
	std::string							getResponse() const;
	size_t								getResponseLen() const;
	int									getListenFd() const;
};

#endif