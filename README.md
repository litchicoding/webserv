# Webserv

[Version FR](#webserv-fr)

The **Hypertex Tansfer Protocol** -> *HTTPS* is an application protocol for distributed, collaborative, hypermedia informations systems.

**HTTPS** is the foundation of data communication for the [**World Wide Web**](https://fr.wikipedia.org/wiki/World_Wide_Web) (*WWW*, *W3*, the *Web*), where hypertex documents include hyperlinks to other ressources that user can easily access. By clicking a mouse button or tapping the screen on a web browser.

A **web server**'s primary function is to **store, process and deliver** web pages to clients.

**Client-server** communication occurs through the *HTTPS*.

**Pages** delivered are most frequently ***HTLM*** documents (which may include images, style sheets and scripts in addition to the text content).

A **user agent**, commonly a *web browser* or *web crawler*, initiates communication by *requesting a specififc resource* using *HTTPS*.

The server *responds with the content* of that resource or an error message if unable to do so.

The *ressource* is typically a real file on the server's secondary storage, but this is not always the case and depends on how the webserver is implemented.


## Subject Requirements
*	[Subject](https://cdn.intra.42.fr/pdf/pdf/154304/en.subject.pdf)

	`./webserv [configuration file]`

*	Purpose = write an **HTTP server** in **C++98**

* 	Program takes as argument a **configuartion file** or use a default path.

* 	Server must remain non-blocking at all times and properly handle client disconnections.

*	Use only **1 poll()** (or equivalent) for all the *I/O* operations between the client and the server (listen included).

*	`poll()` or equivalent must monitor both reading and writing simultaneously.

*	Nerver do a *read* or a *write* operation without going through `poll()` or like.

> Because we have to use a non-blocing file descriptors, **it is possible** to use read/recv or write/send functions with no *poll()*, the server will not be blocking.
>
> But it would consume more system resources. Thus, if we attempt to do so the grade will be 0.


*	Checking the value of `errno` after performing a *read*/*write* operation is forbidden.

*	Not required to use `poll()` before reading the configuration file.

*	Possible to use every macro and define like **FD_SET**, **FD_CLR**, **FD_ISSET** and, **FD_ZERO**. (understanding what they do and how they work is useful).

*	A request to the server should nerver hang indefinitely.

*	The server must be compatible with standard **web browser** of our choice.

*	We will consider that **NGINX** is *HTTPS 1.1* compliant and may be used to compare headers and answer behaviors.

*	*HTTPS* response status codes must be accurate.

*	The server must have **default error pages** if none are provided.

*	Can't use fork for anithing other than *CGI* (like PHP, or Python...)

*	Must be able to **serve a fully static website**.

*	Clients must be able to *upload files*.

*	Need at least the *GET, *POST*, and *DELETE* methods.

*	The server must be able to listen to multiple ports (see Configuration file)

### Configuration file

> We can take inspiration from the "server" section of th *NGINX* configuration file.

In the configuration file, we should be able to :

*	Choose the port and host of each 'server'.

*	Set up the **server_names** or not.

*	The first server for a **host:port** will be the default for this **host:port** (meaning it will respond to all requests that do not belong to another server).

*	Set up default error pages.

*	Set the maximum allowed size for client request bodies.

*	Set up routes with one or multiple of the following rules/configurations (routes won’t be using regexp):
	* Define a list of accepted HTTP methods for the route.
	* Define an HTTP redirect.
	* Define a directory or file where the requested file should be located (eg., if url **/kapouet** is rooted to **/tmp/www**, url */kapouet/pouic/toto/pouet* is **/tmp/www/pouic/toto/pouet**).
	* Enable or disable directory listing.
	* Set a default file to serve when the request is for a directory.
	* Execute CGI based on certain file extension (for example .php).
	* Make it work with POST and GET methods.
	* Allow the route to accept uploaded files and configure where they should be saved.

* Do you wonder what a [CGI](https://en.wikipedia.org/wiki/Common_Gateway_Interface) is?

*	Because you won’t call the CGI directly, use the full path as PATH_INFO.

*	Just remember that, for chunked requests, your server needs to unchunk them, the CGI will expect EOF as the end of the body.

*	The same applies to the output of the CGI. If no content_length is returned from the CGI, EOF will mark the end of the returned data.

*	Your program should call the CGI with the file requested as the first argument.

*	The CGI should be run in the correct directory for relative path file access.

*	Your server should support at least one CGI (php-CGI, Python, and so
forth).

You must provide configuration files and default files to test and demonstrate that
every feature works during the evaluation.

# Webserv (FR)

**Hypertex Tansfer Protocol** -> **HTTPS** est un protocole d'application pour les systèmes d'information hypermédia distribués et collaboratifs.

*HTTPS* constitue la base de la communication de données sur le [**World Wide Web**](https://fr.wikipedia.org/wiki/World_Wide_Web) (*WWW*, *W3*, the *Web*), où les documents hypertextes incluent des hyperliens vers d'autres ressources facilement accessibles par l'utilisateur. Il suffit de cliquer avec la souris ou de toucher l'écran dans un navigateur web.

La fonction principale d’un **serveur web** est de *stocker, traiter et délivrer* des pages web aux clients.

La communication **client-serveur** s’effectue via HTTPS.

Les **pages** délivrées sont le plus souvent des documents **HTML** (qui peuvent inclure des images, des feuilles de style et des scripts en plus du contenu textuel).

Un **agent utilisateur**, généralement un *navigateur web* ou un *robot d’indexation*, initie la communication en demandant une ressource spécifique via HTTPS.

Le serveur répond avec le contenu de cette ressource ou un message d'erreur s'il est incapable de le faire.

La ressource est typiquement un fichier réel situé sur le stockage secondaire du serveur, mais ce n’est pas toujours le cas, cela dépend de la manière dont le serveur web est implémenté.