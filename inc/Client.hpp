#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

class	Server;

typedef struct s_directives t_directives;

using namespace std;

class	Client
{
private:
	/* Client ID **********************************************************************************/
	int					_client_fd;
	int					_listen_fd;
	struct sockaddr_in	_client_addr;
	Server				*_server_config;
	t_directives		*_config;
	/* Request ************************************************************************************/
	bool				_chunked;
	string				_method;
	string				_URI;
	string				_version;
	string				_request;
	map<string, string>	_headersMap;
	string				_body;
	string				_raw_body;
	string				_response;
	string				_root;
	int					_request_len;
	int					_content_len_target;
	size_t				_response_len;

	/* Parsing ************************************************************************************/
	int									handleMethodLine(std::string& line);
	int									handleHeaders(std::string& line);
	void								handleBody(std::string& line);
	std::string							getMIME(std::string& URI);
	bool								URI_Not_Printable(std::string& URI);
	string								urlDecode(const std::string &str);
	int									isRequestChunked();
	int									getCompleteRequest(int epoll_fd);
	int									getChunkedRequest(int epoll_fd);
	int									separateHeadersFromChunkedBody();
	
	/* Response Function ****************************************************************************/

	void								handleError(int code);
	void								sendRedirect(string URI);
	
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
	void								generateDirectoryListing();

	/* HandlePost Function ****************************************************************************/
	
	void								handlePost();
	void								isFilePost();
	void								isDirectoryPost();
	string								findFileName();
	void								uploadFile(const string &filename, int size);
	void								saveData(const string &root,  const string &boundary, int size);
	string								searchBoundary(string &arg);

	/* CGI *********************************************************************************************/

	bool 								isCgi();
	bool								isQueryStringValid();
	bool								isValid();
	char**								buildCgiEnv();
	void								handleCGI();
	void								buildHttpResponseFromCgiOutput(const std::string& cgiOutput);

public:
	Client(int listen_fd, int epoll_fd);
	// Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	/* Member Function ****************************************************************************/
	void								start();
	void								removeRequest();
	void								sendResponse(int client_fd);
	int									parseRawRequest();
	void								buildResponse();
	int	    							request_well_formed_optimized();
	int									requestAnalysis(int epoll_fd);

	/* Setters ************************************************************************************/
	void				setRequest(const string &request, const int &len);
	void				setServerConfig(Server *server_config);
	void				setConfig();

	/* Getters ************************************************************************************/
	int					getClientFd();
	string				getMethod();
	string				getURI();
	string				getVersion();
	string				getRequest();
	Server*				getServerConfig() const;
	string				getResponse() const;
	size_t				getResponseLen() const;
	int					getListenFd() const;
};

#endif