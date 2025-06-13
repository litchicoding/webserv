# Webserv Documentation

**Sommaire** :
* [Server HTTP](#quest-ce-quun-serveur-http-)
* [Connexion Clien-Server avec Sockets](#sockets---connection-between-client-server)
* [Server Configuration](#server-configuration)

## Comment fonctionne un site internet ?

Source : [Comprendre la programmation web](https://www.youtube.com/watch?v=eiM3L4mKUqI)

![architecture-web-app](Media/documentation/schema-archi-app.jpg)

- **Client - Front** : il s'agit de **l'interface directement exploitable par les utilisateurs**, partie émergée de l'iceberg. *IHM* = *Interface Homme Machine*. le code est exécuté par le navigateur de l'utilisateur. Permet une **représentation de notre page internet**. Les données sont mises en forme sous un aspect qui nous permet de les exploiter.

- **Serveur - Back** : la partie traitements, règles métiers et stockage de données, **celle qui régit le fonctionnement de l'application**, la partie immergée de l'iceberg.

- **Navigateurs** : programme qui est capable d'interpréter (*HTML, CSS, Javascript*) et d'afficher une page internet. Interprète des programmes écrits selon les normes *W3C*. Il en existe une grande variété : *Google Chrome, Mozilla Firefox, Microsoft Edge, Safari, Opera*... 
Les navigateurs intègrent également des outils pour aider le développeur dans sa tache : la commande `inspecter` permet entre autre de voir le code source, accéder à la console, voir les erreurs et méthode responsive.

- **HTML (Hypertex Markup Language)** : langage qui va structurer la page web, définit tous les éléments qui vont devoir s'afficher (images, texte, son...), permet de baliser et naviguer entre les pages.

- **CSS (Cascading Style Sheets)** : langage qui permet d'ajouter des éléments de style à la page (couleurs, tailles, alignements, police etc.). Le mot *cascading* vient du fait qu'un élément de style défini pour un élement se répercute sur les éléments qu'il contient. Permet aussi des effets de style comme l'annimation.

- **Javascript** : langage de programmation qui va rendre la page dynamique, grace à des structures de controles (conditions, boucles, fonctions, etc.).

Donc le *HTML*, le *CSS* et le *Javascript* sont considérés comme des langages coté client.
Dans la partie serveur il y a beaucoup plus de liberté du point de vue des langages : *Php, Python, C#, Java, C++, Javascript* (utilisé partie serveur ET client!).
Avec ces langages nous pouvons générer cote serveur des pages interprétables par un navigateur coté client. Par exemple le *Php* va générer du *HTML*.

- **Frameworks** : bibliothèques qui vont faciliter la vie du développeur en leur évitant de réinventer la roue à chaque nouvelle application. Ils fournissent des **outils pour une base commune et servent un grand nombre d'applications différentes**. Ils sont écrits dans différents langage de programmation et peuvent etre coté serveur et coté client.
	- se distinguent des *bibliothèques* par leur aspect non spécialisé : les *frameworks* permettent de créer n'importe quel site web.
	- exemple de **frameworks par langage coté client** : *Javascript* -> *React, Ember.js, svelte, Angular, vue.js*
	- exemple de **frameworks par langage coté serveur** : 
		- *Php* -> *Symfony, Laravel, Zend*
		- *Python* -> *Django, Flask*
		- *C#* -> *ASP, .NET*
		- *Java* -> *J2EE*
		- *Ruby* -> *Ruby on Rails*
		- *Javascript* -> *Node.js, express.js, Meteor*

- **Bibliothèques** : ensemble d'outils, spécifique à une tache. 
	- *JQuerry* : permet de parcourir facilement le *DOM* et contient des fonctionnalités d'animation.
	- *Ajax* : permet de modifier l'affichage d'une page web sans avoir à la recharger.
	- *Bootstrap* : sert à la création de design pour HTML, CSS et avec des extensions JS.

- **Data Base** : à pour fonction de **stocker** des informations. La Data base se trouve du coté serveur, ce qui empeche le client d'avoir accès à toutes les données. Sa gestion se fait à l'aide d'un langage spécifique.
	- *Les bases relationnelles* (intègrent des outils pour s'assurer de la cohérence des données) : *MySQL, SQL Server, Oracle, MariaDb, PostGRESQL*.
	- *Les Bases NoSQL* (plus rapides mais vérifiant moins de choses que les bases SQL) : *MongoDb, Apache Cassandra, Apache Hbase, RavenDB*.

- **ORM (Object-relational mapping)** : composant logiciel qui se glisse entre un programme et une base de données. S'occupe de gérer les problèmes d'interface. Utile aussi pour créer la base de donnée (porte d'entrée vers la data base == interface). Les *ORMs* sont souvent intégrés aux *frameworks* :
	- *Javascript* : *Knex.js, Mongoose*
	- *Php* : *Doctrine, Eloquent*
	- *Python* : *SQLAlchemy, MongoEngine*
	- *Java* : *Hibernate*
	- *C#* : *Dapper*

### Le matériel physique d'Internet

Tout système connecté au reseau Internet est un **hote**. On peut diviser les *hotes* en deux catégories : clients et serveurs.

Le *client* peut etre un ordinateur, smartphone, console de jeu ou tout autre objet connecté.
Le *serveur* est une machine qui stocker et distribue des pages de sites web, diffuse des vidéos en streaming, relaie les emails etc.

Quand un *hote* envoie des données à un autre : il découpe les données et y ajoute quelques octets en tete. Le resultat est une suite de **paquets** qui est expédiée à travers le réseau jusqu'à l'*hote* de destination.

Le destinataire doit ensuite rassembler les *paquets* et reconstituer les données après réception.

Les *hotes* sont liés entre eux à l'aide de différents **supports physiques** sur lesquels peuvent circuler tous les bits d'un *paquet*.

Ces *supports physiques* peuvent etre des lignes coaxiales, des cables de cuivree, fibres optiques ou des ondes radio.

Les *paquets* doivent passer par des **commutateurs réseau** (=**switches**) et des **routeurs**. Ces deux appareils ont pour unique but de recevoir des *paquets* depuis leur ligne d'entrée et de les transférer via leur ligne de sortie la plus appropriée.

Chaque *hote*, *routeur* ou *commutateur* est un **noeud du reseau**. Cette suite de relais par lequel passe un *paquet* pour arriver à sa destination est ce qu'on appelle le **routage**.

Pour mieux comprendre :
* [Routage (adresses IPv4 et submask)](https://www.codequoi.com/adresses-ipv4-routage-et-masques-de-sous-reseau/)


# Qu'est-ce qu'un serveur HTTP ? 

Sources :
* [Glossaire MDN : définitions des termes du Web](https://developer.mozilla.org/fr/docs/Web/HTTP)
* [Documentation pour Webserv](https://hackmd.io/@fttranscendance/H1mLWxbr_)
* [Comprendre l'architecture en couches](https://www.codequoi.com/adresses-ipv4-routage-et-masques-de-sous-reseau/)

### HTTP (Hypertex Transfert Protocol)
Est un protocole de communication informatique basé sur le modèle client-serveur.

Ce protocole est le fondement du *World Wide Web*, il permet de ***transporter toutes sortes de données*** (des pages web, fichiers, streaming).

Il permet une communication externe (Internet) ou interne (intranet) entre plusieurs ordinateurs.

**Serveur HTTP** : un serveur informatique répondant à des requetes sous la forme de relation client-serveur. Il peut prendre la forme d'un ordinateur contenant ***un logiciel qui va répondre à des requetes de protocole HTTP***.

Il est pensé pour etre disponible en permanence à recevoir et répondre à des requetes simultanément venant des clients.

**HTTPS != HTTP** -> HTTPS a une nouvelle surcouche sécurisée, qui permet de chiffrer les communications entre le client et le serveur pour protéger contre l'analyse des communications (avec des données sensibles : mot de passe, codes bancaires).

***Apache*** et ***Nginx*** sont les deux logiciels les plus utilisés en tant que serveur *HTTP*. Mais il est préférable de se baser sur le comportement (configuration, syntaxe et mot-clés) de *Nginx*.

### Anatomie d'une requete HTTP

Source : 
* [Protocole HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
* [Article explicatif](https://ege-hurturk.medium.com/creating-your-own-http-server-part-i-c1d567735af2)

![http-request-message](Media/documentation/request_http.png)

Une requete HTTP est un message suivant un certain format :

- 1. **Request line** = *method + target request + protocol + CRLF*
	- `<method>` : Indique le but de la requete. Par exemple `GET` signifie que le client attend une ressource en retour. `POST` indique que le client envoie des donnée au serveur. [(Liste complète des méthodes ici)](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Methods).
	- `<request-target>` : Attend `URI`, une chaine qui fait référence à une ressource, peut etre une **URL**, chemin de  la ressource sur le serveur, exemple : */index.html*, un seul slash "/" designe *index*.
	- `<protocol>` : Attend la version de *HTTP* voulue.

- 2. **Header** = Zéro, un ou plusieurs messages indicatifs sur la requete + CRLF

- 3. CRLF (empty line)

- 4. **Optional message body** = Contient des données associées avec la requete, dans le cas d'un `POST` par exemple on signifie ce que l'on souhaite transmettre au serveur comme data.

CRLF = saut de ligne comme ca `\r\n`.

### Anatomie d'une réponse HTTP

![request-response](Media/documentation/request_response.webp)

Après avoir recu une requete HTTP, le serveur écrit et envoie une réponse :

- **Status line** =  `<HTTP version>` + `<status code>` + `<status message>`
	- [status message & code HTTP list](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)
- Headers + CRLF
- CRLF
- Message body

### Etapes du processus HTTP

***The client enters the *URL* and the *browser* constructs an HTTP request.***

Lorsqu'un client souhaite communiquer avec le serveur, voici les différentes étapes :
-	1. **Opening a TCP connection:** la connection doit etre établie pour transmettre/recevoir les requetes/réponses depuis le serveur. 
-	2. **Send an HTTP request:** Le client envoie une requete HTTP à travers la connexion *TCP* suivant le bon format de requete.
-	3. **Receive an HTTP response:** Le serveur interprete la requete HTTP et prepare une reponse, puis l'envoie selon le bon format.
-	4. **Close the TCP connection:**

### Web Request flow
Source : [video here](https://www.youtube.com/watch?v=hWyBeEF3CqQ)

- 1. entering the URL
	- User is entering the URL in the browser
	- Browser quickly parse the URL
- 2. DNS Resolution
	- returns the IP address of the domain
- 3. Establishing the Connection
	- with an IP address, the browser can now connect to a Web Server
	- the connection is made with TCP (Transmission Control Protocol) or UDP (User Datagram Protocol)
	- The client sends a SYN (Synchronize packet) packet to the server to request a connection.
	- The server answers with a SYN/ACK (Sync Acknowledge Packet).
	- At this point TCP connection is done.
- 4. Sending th HTTP Request
	- send a GET request to fetch HTML Document from server, according to the HTTP request
	- HTTP request contains many important data in headers like the user-agent, describes which browser is being used. Or can contains Cookies, website data stored on our local machine.
	- caching is re-susing data that was previously dowloaded
- 5. The server recieves the request
	- the server uses its data base, call API or uses scripts to customize a response to the request.
- 6. Backend Processing (optional)
- 7. Generating the response
	- Prepares a HTTP response (status code, mesasge, headers with content, the body contains the html page)
- 8. Sending the HTTP Response
	- Responses goes through routers to the machine
- 9. Receiving the response
	- Load the web page nad all the data associated (content, cookies, policies...) because the data arrives in packets... and the browser renders  the page and displays it
- 10. HTML Parsing
	- DOM = document object model.
- 11. and so on with the engine details

# Sockets - connection between Client-Server

- Source :
* [Programmation réseau via socket](https://www.codequoi.com/programmation-reseau-via-socket-en-c/)
* [Video Sockets d'1h](https://www.youtube.com/watch?v=oYBgV474Udc)
* [Socket Programming GeeksForGeeks](https://www.geeksforgeeks.org/socket-programming-cc/)
* [Inner workings of the Webserver](https://hackmd.io/@laian/SJZHcOsmT#Sockets-and-Useful-Network-Functions)
* [Handling sockets with C++](https://ncona.com/2019/04/building-a-simple-server-with-cpp/)
* [Video d'1h - Network Programming - C++ Sockets](https://www.youtube.com/watch?v=gntyAFoZp-E)

The connection between clients and servers are facilitated by sockets, which are the *communication link between two processes on a network*.

A ***socket is a file descriptor created using this function*** :

```cpp
int	socket(int, domain, int type, int protocol);
```
- `domain` = refers to the communication domain. [Domain list here](https://linux.die.net/man/2/socket)
- `type` = type of socket.
- `protocol` = refers to a particular protocol to be used with sockets, usually is **0**.

#### Types of sockets

- **Stream Sockets** -> `SOCK_STREAM`
-> are reliable two-way connected communication streams which uses TCP. These sockets are usually used by HTTP when speed is not the priority, but data quality is.

- **Datagram Sockets** -> `SOCK_DGRAM`
-> are connectionless sockets that use the UDP (User Datagram Protocol). So when data is sent, it may not arrive. These sockets are commonly used by games/video/audio where speed is priority.

Once we have a socket descriptor, we need to ***bind it to a port on the computer*** with:
```cpp
int	bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

## Polling - Server Non-Blocking

- Source : [polling](https://hackmd.io/@laian/SJZHcOsmT#Polling)

When we try to read from a socket without any data into it, it is **blocking the server**.

**Polling** is telling the system to let us know which sockets have the data ready, and we can perform operations on them.

```cpp
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

See more in article above.

## Qu'est-ce que le CGI ?

**CGI (Common Gateway Interface)** : interface standardisée qui permet aux serveurs web d'exécuter des programmes externes pour traiter les requetes HTTP des utilisateurs cote client. 

Le *CGI* permet donc un échange de données entre des applications externes et les sevreurs web.

Fonctionnement du CGI :
- **Interaction Utilisateur** : Lorsqu'un utilisateur remplit un formulaire sur une page web et le soumet, les données du formulaire sont envoyées au serveur web via une requete HTTP.
- **Traitement par le serveur** : Le serveur recoit cette requete et lance un programme CGI, souvent ecrit avec un langage de script comme Python.
- **Execution du script CGI** :Le script CGI traite les données recues, effectue les operations necessaires (comme l'accès à une base de données) et génère une réponse.
- **Reponse au client** : Le serveur renvoie la réponse générée par le script CGI u client, généralement sous forme d'une page HTML.


# Server Configuration

Sources : 
-	global understanding :
	* [Understanding Nginx Server and Location Block Selection Algorithms](https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms)
	* [Nginx documentation](https://nginx.org/en/docs/index.html)
-	listen directive :
	* [How Nginx processes a request](https://nginx.org/en/docs/index.html)
	* [Listen documentation](https://nginx.org/en/docs/http/ngx_http_core_module.html#listen)
-	autoindex directive :
	* [Explaining the Nginx directory index file](https://www.keycdn.com/support/nginx-directory-index#:~:text=By%20default%2C%20Nginx%20tries%20to,is%20if%20it%20has%20permissions)
-	server_name directive :
	* [Server names documentation](https://nginx.org/en/docs/http/server_names.html)

Configuration is based on *Nginx*'s server configuration format.

-	Les instructions de configurations sont dans des blocks qui suivent une structure hierarchique.

-	Le fichier peut contenir plusieurs **server blocks** contenants des **directives** et des **location blocks** qui eux aussi contiennent des directives.

Nginx lance un ***processus qui determine quel block de configuration devrait etre utilisé pour gerer une requete***.

-	L'administrateur configure souvent plusieurs *server blocks* et décide quel block devra gérer telle connexion selon le **doamin name**, le **port** et l'**IP address**.

-	Le *location block* est utilisé pour définir comment Nginx doit traiter les demandes de différentes ressources et **URI** pour le serveur parent. L'espace *URI* peut être subdivisé comme l'administrateur le souhaite à l'aide de ces blocks.


```cpp
server {
	...
	location <URI> {
		...
	}
}
```

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

#### Parsing `server_name` to choose a match

### 

### Tutoriels, guides et documentation de références

https://www.youtube.com/watch?v=-v5vCLLsqbA

https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf

https://github.com/Kaydooo/Webserv_42

https://bousk.developpez.com/cours/reseau-c++/TCP/08-premier-serveur-mini-serveur/

https://www.youtube.com/watch?v=s3o5tixMFho

https://github.com/Dungyichao/http_server

https://en.wikipedia.org/wiki/HTTP

http://vidalc.chez.com/lf/socket.html

https://github.com/Tablerase/42_WebServ/wiki/Home/ed1da06241dbecd510ffb702094d7af7fff191bf

https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms

https://www.youtube.com/watch?v=P9csgxBgaZ8

https://www.youtube.com/watch?v=N49UyTlUXp4

https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines C++

https://www.ibm.com/docs/fr/was-nd/8.5.5?topic=icwspi-configuring-web-server-application-server-profile-same-machine


https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms

https://developer.mozilla.org/fr/docs/Web/HTTP/Guides/Overview

### Ressources en brol
* [https://www.youtube.com/watch?v=9J1nJOivdyw](https://www.youtube.com/watch?v=9J1nJOivdyw)
* [https://hackmd.io/@laian/SJZHcOsmT](https://hackmd.io/@laian/SJZHcOsmT)
* [https://www.youtube.com/watch?v=YwHErWJIh6Y](https://www.youtube.com/watch?v=YwHErWJIh6Y)
* [https://www.youtube.com/watch?v=gntyAFoZp-E](https://www.youtube.com/watch?v=gntyAFoZp-E)
* [https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7)
* [https://developer.mozilla.org/fr/docs/Web/HTTP](https://developer.mozilla.org/fr/docs/Web/HTTP)
* [https://fr.wikipedia.org/wiki/Hypertext_Transfer_Protocol](https://fr.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
* [https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm)
* [https://ncona.com/2019/04/building-a-simple-server-with-cpp/](https://ncona.com/2019/04/building-a-simple-server-with-cpp/)
* [https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)
* [https://www.geeksforgeeks.org/socket-programming-cc/](https://www.geeksforgeeks.org/socket-programming-cc/)
* [https://ege-hurturk.medium.com/creating-your-own-http-server-part-i-c1d567735af2](https://ege-hurturk.medium.com/creating-your-own-http-server-part-i-c1d567735af2)
* [https://www.youtube.com/watch?v=hWyBeEF3CqQ](https://www.youtube.com/watch?v=hWyBeEF3CqQ)
