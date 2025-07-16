# Server Configuration File

Sources : 
-	global understanding :
	* [Understanding Nginx Server and Location Block Selection Algorithms](https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms)
	* [Beginner's guide Nginx](https://nginx.org/en/docs/beginners_guide.html)
	* [Nginx documentation](https://nginx.org/en/docs/index.html)
	* [Basic rules](https://github.com/trimstray/nginx-admins-handbook/blob/master/doc/RULES.md#beginner-organising-nginx-configuration)
	* [nginx configuration](https://www.solo.io/topics/nginx/nginx-configuration)
	* [Webserv context configuration](https://github.com/tdameros/42-webserv/blob/main/docs/config_file.md)

Le format et les règles de configurations sont basés sur le comportement du serveur *Nginx*.

-	Les instructions de configurations sont dans des blocks qui suivent une structure hierarchique.

-	Le fichier peut contenir plusieurs **server blocks** contenants des **directives** et des **location blocks** qui eux aussi contiennent des directives.

Nginx lance un ***processus qui determine quel block de configuration devrait etre utilisé pour gerer une requete***.

-	L'administrateur configure souvent plusieurs *server blocks* et décide quel block devra gérer telle connexion selon le **domain name**, le **port** et l'**IP address**.

-	Le *location block* est utilisé pour définir comment Nginx doit traiter les demandes de différentes ressources et **URI** pour le serveur parent. L'espace *URI* peut être subdivisé comme l'administrateur le souhaite à l'aide de ces blocks.


```cpp
server {
	...
	location <URI> {
		...
	}
}
```

## listen

*	[Listen Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#listen)

***Sets the address and port for IP, or the path for a UNIX-domain socket on which the server will accept requests.***

```
Syntax:		listen address[:port] [default_server]
Default:	listen *:80;
Context:	server
```
- `localhost` can be given as an address.
- if only `adress` is given, the port `80` is used.
- if the directive is not present then :
	- if server is runing with `root` = `*:80`; else `*:8000`
- if `default_server` is present so this become the main server, otherwise it's the first server with the `adress`:`port` pair.

Exemple de directives `listen` possibles :
```
listen	127.0.0.1:8000;
listen	127.0.0.1;
listen	8000;
listen	*:8000;
listen	localhost:8000;
```

## server_name

* [server_name Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#server_name)

***Sets names of a virtual server.*** The first name becomes the primary server name.

```
Syntax:		server_name name ...;
Default:	server_name "";
Context:	server
```

- Server names can include an asterisk replacing tthe first or last part of a name :
```
server {
	server_name	example.com *.example.com www.example.*;
}
```

- It is possible to use regular expression preceding the name with a tilde. ex: `~^www\d+\.example\.com$;`
- If the directive's paramater is set to `$hostname`, the machine's hostanme is inserted.

## location

*	[location Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#location)

***Sets configuration depending on a request URI.***

```
Syntax:		location [ = | ~ | ~* | ~^ ] uri { ... }
			location @name { ... }
Default:	----
Context:	server, location
```

Se renseigner pour savoir si le sujet exige de pouvoir imbriquer les location block... et doit on implementer = ~ etc!!!

## error_page

*	[error_page Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page)

***Defines the URI that will be shown for the specified errors.***

```
Syntax:		error_page code ... [=[response]] uri;
Default:	-----
Context:	server, location
```

- A `uri` value can contain variables. This causes an internal redirect to the specified uri with the client request method changed to “GET”.
- It is possible to change the response code to another using the “=response” syntax.
- If uri processing leads to an error, the status code of the last occurred error is returned to the client.

## root

*	[root Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#root)

***Sets the root directory for requests.***

```
Syntax:		root path;
Default:	root html;
Context:	server, location
```

- The path value can contain variables.
- A path to the file is constructed by merely adding a URI to the value of the root directive.

## return

*	[return Documentation](https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return)

***Stops processing and returns the specified code to a client.***

```
Syntax:		return code [text];
			return code URL;
			return URL;
Default:	----
Context:	server, location
```

## index

*	[index Documentation](https://nginx.org/en/docs/http/ngx_http_index_module.html#index)

***Defines files that will be used as an index.***

```
Syntax:		index file ...;
Default:	index index.html
Context:	server, location
```

- The file name can contain variables.
- Files are checked in the specified order. The last element of the list can be a file with an absolute path.

## allow_methods

```
Syntax:		allow_methods method ...;
Default:	allow_methods GET POST DELETE;
Context:	server, location
```

## client_max_body_size

*	[Documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#client_max_body_size)

***Sets the maximum allowed size of the client request body.***
If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size.

```
Syntax:		client_max_vody_size size;
Default:	client_max_body_size 1m;
Context:	server, location
```

## autoindex

*	[autoindex Documentation](https://nginx.org/en/docs/http/ngx_http_autoindex_module.html)

***Enables or disables the directory listing output.***

```
Syntax:		autoindex on | off;
Default:	autodindex off;
Context:	server, location
```

## cgi_param 

must get information


### Comment décider quel block traitera quelle Request ?

Étant donné que Nginx permet à l'administrateur de définir plusieurs *server block** qui fonctionnent comme des instances de ***serveurs web virtuels distincts***, il a besoin d'une procédure pour déterminer lequel de ces blocs de serveurs sera utilisé pour répondre à une demande.

Pour ce faire, il utilise un ***système défini de vérifications*** afin de trouver la meilleure correspondance possible.

Les principales directives du bloc serveur dont se préoccupe Nginx au cours de ce processus sont la directive **listen** et la directive **server_name**.

#### Parsing `listen` to find possible matches

Cette directive définit généralement l'IP address et le port auquel le serveur répondra. Mais si rien n'est spécifié alors **par défaut le serveur se voit attribuer :** `0.0.0.0:80` (ou `0.0.0.0:8080` si n'est pas exécuté en mode `root`)

**listen** peut etre définit comme : 
-	un combo IP/port 
-	IP address unique sans port définit = donc port par défaut 80
-	port unique sans IpP = donc par défaut 0.0.0.0 
-	le chemin vers un socket Unix (have implications when passing requests between different servers)

Tout d'abord, Nginx examine l'*IP address* et le *port* de la request. Il les compare à la directive `listen` de chaque serveur afin de dresser une liste des *server blocks* susceptibles de répondre à la request.

- **Step 1**: Nginx traduit toutes les directives *listen* "incomplètes" en remplacant les valeurs manquantes par leurs valeurs par défaut afin que chaque bloc puisse etre évalué par son IP address et son port.
- **Step 2**: Nginx tente de rassembler une liste des server blocks  qui correspondent le plus précisément à la request, en se basant sur l'IP + le port. Le port doit etre exactement pareil pour matcher. Et pour une addresse IP spcifique de la request, un bloc avec 0.0.0.0 ne sera pas choisi il existe une autre addresse IP qui match spécifiquement.
- **Step 3**: Si il y a seul un *match* spécifique, c'est ce block qui est choisi. Si il y a plusieurs match alors Nginx commence à évaluer la directive `server_name`

#### Example

```cpp
server {
	listen 192.168.1.10;
	...
}

server {
	listen 80;
	server_name example.com;
	...
}
```
-	Request : example.com is hosted on port 80 of 192.168.1.10
-	After translation : 
	-	first block = 192.168.1.10:80
	-	second block = 0.0.0.0:80
-	**First block is selected** because specific match with IP/port, even if the second block matches with the server_name it's not selected.

#### la suite du processus reste à étudier....