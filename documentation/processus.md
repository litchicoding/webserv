# Processus du serveur 

Description des différentes étapes du processus pour créer un serveur HTTP.

Source :
*	[Article sur la prog d'un serveur HTTP](#file:///C:/Users/User/Downloads/HTTP%20Server_%20Everything%20you%20need%20to%20know%20to%20Build%20a%20simple%20HTTP%20server%20from%20scratch_pdf2.pdf)

Étapes :

* [Configuration du serveur](#configuration-du-serveur)
* [Établir une conexion TCP = Client-Server](#etablir-une-connexion-client-server-tcp)
* [Traiter la requete - Théorie](#traiter-la-requete-http---théorie)
* [Traiter la requete - Pratique](#traiter-la-requete-http---pratique)

-Accepter les connexions sur chaque socket.

-Lire "Host:" et "path" pour déterminer quel server et quel location appliquer.

-Traiter la requete et répondre.

-Gérer chaque requête de manière non-bloquante via epoll().

## Configuration du serveur

* [Notes et documentation sur le fichier de configuration](configuration.md)

Commencer par analyser le fichier de configuration, ce qui va nous aider à déterminer les paramètres de un ou plusieurs **virtual-server**.

Pour chaque *server block* on instancie un object de la classe *Server* qui contiendra tout un tat d'informations nécessaires pour ensuite établir une connexion avec le client, recevoir une http request, la traiter et y répondre correctement.

Chaque bloc server doit être associé à une ou plusieurs location!

À creuser :
->	Principe de serveur par défaut, c'est à dire avec un port/adresse mais aussi d'autres valeurs par défaut. Le fait est qu'un server peut etre définit explicitement comme serveur par défaut, donc propablement qu'on doit pas en set un en plus si c'est spécifié dans le fichier de config.


#### Exemple contenu du fichier de configuration :
```
server {
    listen 80;
    server_name example.com;

    location / {
        root /var/www/html;
        index index.html;
    }

    location /images/ {
        root /data/images;
    }
}
```

#### Ce que ce fichier dit :
📡 listen 80; → écoute sur le port 80

🌐 server_name example.com; → ce bloc s’applique à ce nom de domaine

📁 location / → quand un utilisateur accède à /, on va chercher dans /var/www/html

📄 index index.html; → si l'utilisateur demande /, on sert /var/www/html/index.html

🖼️ location /images/ → les URLs comme /images/cat.jpg chercheront le fichier /data/images/cat.jpg


### Tokenisation

à compléter...

### Règles de grammaires

à compléter...

## Etablir une connexion client-server (TCP)

**TCP** = Transmission Control Protocol

Le protocole *TCP* est une norme de communication qui permet aux programmes informatiques d'échanger des messages sur un réseau. Donc permet la transmission de paquets.

Donc l'objectif est d'établir une connexion entre le client et le serveur.
Pour cela on doit créer des sockets et les configurer, c'est ainsi que nous pourrons recevoir et envoyer des messages sur le reseau.

### Créer des sockets

```cpp
int server_fd = socket(domain, type, protocol);
```

- **domain** => dans quel domaine de communication (comprendre addresse IP) le socket devra etre crée. À compléter avec un flag **address family** : `AP_INET` (IPv4), `AF_INET6` (IPv6), `AF_UNIX` (local channel, similar to pipes), `AF_ISO` (ISO protocols) ou `AF_NS` (Xerox Network Systems).
- **type** => type du service, `SOCK_STREAM` (virtual circuit service), `SOKC_DGRAM` (datagram service) ou `SOCK_RAW` (direct IP service).
- **protocol** => indique le protocol auquel doit etre soumis le socket.

#### exemple :
```cpp
#include <sys/socket.h>
...
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("cannot create socket");
	return 0;
}
```

### Identifier (lier) un socket

Il est question ici d'assigner une adresse de transport au socket, donc le numéro d'un **port** et une **adresse IP**. On appelle cette opération **bind**.

```cpp
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
```

- **socket** => ce qui nous permet d'identifier le socket en question donc *server_fd* (le file descriptor retourné par la fonction sokect()).

- **sockaddr** => structure qui permet au système de lire les premiers bits qui identifie l'*adress family*, qui elle-meme permet d'identifier quelle variante de structure de *sockaddr* utiliser (quels élements vont servir dans ce contexte (exemple = contexte IPv4 ou IPv6 ?)). 

- **address_len** => selon le type de transport (adresses IP) utilisé peut différer donc ce paramètre permet de spécifier la taille de la structure : `sizeof(sockaddr_in)`.

Dans notre contexte on utilise `structaddr_in` qui est définie avec `#include <netinet/in.h>`. Il faut déclarer et remplir la structure avant de faire appel à la fonction *bind()*.
```cpp
struct sockadrr_in
{
	_uint8_t		sin_len;
	sa_family_t		sin_family;
	in_port_t		sin_port;
	struct in_addr	sin_addr;
	char			sin_zero[8];
}
```

- **sin_family** => l'**address family** utilisée pour set up le socket. Dans notre cas `AF_INET`.

- **sin_port** => Le numéro du *port* (*transport address*). On peut explicitement assigner un port ou laisser l'OS en assigner un lui meme. Pour le *cas du client* qui ne va pas avoir à recevoir les connexions, en principe on peut laisser l'OS décider du port et en choisir un disponible, pour cela on entre en argument 0. Pour le serveur en général on choisit un nombre en particulier, puisqu'on doit savoir quel port écouter pour recevoir une demande d'un client.

- **sin_addr** => L'addresse IP pour ce socket. La machine a une adresse IP pour chaque interface de réseau. Par exemple, si la machine a une connexion Wi-Fi et une pour Ethernet alors la machine a deux adresse IP,  une pour chaque interface. La pluspart du temps nous n'avons pas besoin de renseigner une adresse en particulier et laisser l'OS choisir. Dans ce cas on utilise une adresse spéciale `0.0.0.0`, définie avec la macro `INADDR_ANY`.

#### Le code pour effectuer l'opération *bind()*
```cpp
#include <sys/socket.h>
...

struct sockaddr_in address;
const int PORT = 8080;	// Où les clients s'adressent

memset((char *)&address, 0, sizeof(address));
memset(address.sin_zero, '\0', sizeof(address.sin_zero));
address.sin_family = AF_INET;
address.sin_port = htons(PORT);	// converts a short int, here address
address.sin_addr.s_addr = htonl(INADDR_ANY); // converts a long int, here address

if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
	perror("bind failed");
	return 0;
}
```

### Écouter des connexions entrantes

Avant qu'un client puisse se connecter au serveur, le serveur devrait avoir un socket configuré et pret à pouvoir accepter des connexions.

L'opération `listen` établit qu'un socket est pret et capable d'accepter des connexions entrantes.

```cpp
int listen(int socket, int backlog);
```

- **backlog** => définit le nombre maximum de connexions qui peuvent rester en attente avant d'etre tout simplement refusées.

L'opération `accept` récupère la première connexion en liste d'attente et crée un nouveau socket pour cette connexion.

Le socket qui à été configuré à l'origine est utilisé *seulement* pour accepter des connexions, par pour échanger des données.
 
Par défaut, une opération socket est asynchrone, ou bloquante, et `accept` va bloquer jusqu'à ce qu'une connexion soit présente dans la file d'attente des écoutes.

```cpp
int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
```

- **socket** => ici c'est le socket identifié jusqu'alors avec *bind* et *listen*.
- **address** => Une structure *sockaddr* vierge. Ce qui va nous permettre si besoin d'analyser ensuite l'adresse et le numéro de port du socket qui s'est connecté.
- **address_len** => `sizeof(client_sockaddr)`.

```cpp
if (listen(server_fd, 3) < 0) {
	perror("In listen");
	exit(EXIT_FAILURE);
}

if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
	perror("In accept");
	exit(EXIT_FAILURE);
}
```

### Recevoir et envoyer des messages

Après avoir connecter des sockets entre le client et un serveur (***connexion lorsqu'on visite l'adresse IP du serveur dans un navigateur***).

On peut utiliser par exemple `read` et `write` avec des sockets.

#### Exemple :

```cpp
char buffer[1024] = {0};

int	bytes_readed = read(new_socket, buffer, 1024);
std::cout << buffer << std::endl;
if (bytes_readed < 0)
	std::cout << "No bytes are there to read" << std::endl;

char *hello = "Hello from the server";
write(new_socket, hello, strlen(hello));
```

***Tout le travail du serveur HTTP réside dans le contenu présent dans la variable `hello`.***

### Fermer le socket

Quand la communication prend fin, on ferme le socket du client, de la meme facon qu'on fermerait un fichier.

```cpp
close(new_socket);
```

## Traiter la requete HTTP - Théorie

Nous sommes à présent au stade où :
- Le client (par exemple un navigateur web, ou un terminal) envoie une requete HTTP au serveur HTTP.
- Le server recoit et proccesse la demande et envoie une reponse HTTP au client.

### La requete coté Client HTTP

Le client à besoin de se connecter au serveur à tout moment. Le serveur ne pas se connecter au client, c'est donc le client qui *initie* la connexion.

Ainsi en tant que client on peut taper des **URL/Address** d'un site dans un navigateur.

`http://www.example.com:80/index.html` => *URL* (Universal Resource Locator)

Pour afficher la page, le navigateur va chercher le fichier `index.html` chez un serveur web.

Ainsi, si vous tapez `www.example.com` dans le navigateur web, celui-ci recompose l'URL/l'adresse comme suit : `http://www.example.com:80`. C'est ce que les navigateurs web envoient aux serveurs chaque fois que l'on navigue sur les pages internet. 

Si le serveur est configuré pour certaines pages par défaut, alors par exemple une page peut s'afficher lorsqu'on visite un dossier sur le serveur. Cette page en question est détérminée par le nom du fichier. Certains serveurs ont `public.html` et d'autres configurés avec `index.html`.

### HTTP Methodes

**GET** est la méthode par défaut utilisée par HTTP.

Il existe en tout 9 méthodes HTTP :

- **GET** -> Aller chercher une URL.
- **POST** -> Envoie des données à une URL et recoit une réponse en retour.
- **DELETE** -> Supprimer une URL **GET** et **POST** généralement
- **HEAD** -> Aller chercher une information à propos d'une URL.
- **PUT** -> Stocker une URL.

### La requete coté serveur HTTP

Lorsqu'on recoit une requete il faut ensuite y fournir une réponse appropriée.

Dans la requete de client on retrouve des **headers** et on doit en fournir en retour. Car le navigateur s'attend à recevoir une réponse sous le format HTTP et prévu pour la requete en question.

Le format HTTP est donc soumis à des règles établies par les **RFC documents**.

Pour un exemple simple on va renvoyer au serveur un message : "Hello from the server".

Pour cela on construit le **Header**, puis on insére une **ligne vide** ("\r\n"), puis la réponse (message ou données) qu'on appelle **body**.

Il existe différents *Headers* dans HTTP, on peut consulter par exemple `RFC 7230`, `RFC 7231`, `RFC 7232`, `RFC 7233`, `RFC 7234`, `RFC 7235`.

#### Exemple :

```html
Header	| HTTP/1.1 200 OK\r\n
		| Date: Fri, 16 Mar 2018 17:36:27 GMT\r\n
		| Server: *Le nom du serveur*\r\n
		| Content-Type: text/html;charset=UTF-8\r\n
		| Content-Length: 1846\r\n
\r\n
Body	| <html ...>
		|...
		| </html>
```

Donc le contenu de `hello`, la chaine de char envoyée avec `write()` pourrait devrait etre :
```cpp
char	*hello = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\nHello from the Server!";
```

Les *Headers* exigent au minimum ces 3 renseignements :
- Ligne `HTTP/1.1 200 OK` -> indique dans l'ordre la **version HTTP**, le **status code** et le **status message**.
- Ligne `Content-Type`
- Ligne `Content-Length` -> le navigateur ne lit pas plus que ce que l'on a indiqué.

Donc nous devons nous assurer du nombre de bytes que l'on envoie dans la partie *Body* et le renseigner dans `Content-Length` ainsi que du type de donnée renvoyé et le spécifier dans `Content-Type`.

### Traiter une  requete et y répondre

La première chose à considérer c'est la 1ère ligne du *Header* : la **request line**, ou **request header**.

Donc si on recoit `GET /info.html HTTP/1.1`, nous devons chercher le fichier `info.html` dans le dossier courant, car le slash au debut du path `/` signifie que ***le fichier se trouve dans le dossier root*** du serveur. Dans un autre cas comme `/message/info.html` on chercherait dans le dossier "message" pour trouver le fichier "index.html"

Il y a beaucoup de cas différents à considérer, en voici un exemple :
- Est-ce que le fichier (page web) est présent ?
- Ou le fichier est-il absent ?
- Et s'il existe, le client a-t-il la permission d'accéder à ce fichier ?

Selon la réponse à ces questions on va compléter la réponse HTTP :
-	On selectionne le *status code* adéquat. 
-	On ouvre le fichier, on stocke les données dans un variable, ce qui permet de renseigner *Content-Type*.
-	On compte les bytes lus puis on assigne la valeur à *Content-Length*.
-	Enfin pour conclure le *Header* on ajoute une ligne vide à la fin.
-	Si besoin de contenu dans le *Body* on concatène les données à la réponse.

Lorsque que la réponse est ***compléte et conforme au format HTTP*** on peut l'envoyer au client.

### Status Code et Status Message

Les **status code** sont emis par le serveur en réponse à une demande/requete faite par un client. Cela inclut les codes provenant de `IETF Request Comments (RFCs)`, d'autres spécifications et puis des codes utilisés dans des applications HTTP.

Le premier chiffre du *status code* indique l'une des 5 classes standards de réponse. Les phrases du message indiquées sont typiques, mais toute alternative lisible par l'homme peut être fournie. Sauf indication contraire, le code d'état fait partie de la norme HTTP/1.1 (RFC 7231).

Donc si on ne trouve pas un fichier demandé par le client ou si il n'a pas la permission on renvoie le *status code* approprié.

## Traiter la requete HTTP - Pratique

Voici une tentative de lister toutes les étapes du **parsing de la requete** afin de **convenir d'une réponse** adéquate :

-	Créer un buffer assez large et l'initialiser à zéro/NULL.
-	Stocker la requete dans le buffer et concerver le nombre de bytes lus avec `read()`, pour connaitre la taille de la requete.
-	Analyser la *Request Line* :
	-	Récupérer le type de méthode.
	-	Récupérer le path et identifier le type d'extension du fichier.
	-	Que doit on faire avec le protocol, on doit vérifier qu'il match ??? (à vérifier!)

Si erreur :
-	Chercher si le code de l'erreur correspond à des pages stockées en interne et configurées dans le serveur avec error_page.
-	Pour POST, limiter la taille du body avec client_max_body_size.


https://nginx.org/en/docs/http/request_processing.html

Comment Nginx process la selection de paramétres :
-	Comparer "Host" avec le nom du serveur. si `server_name == ""` alors le virtual-server peut match avec une request sans la mention `Host` dans son header. Sinon on cherche un match.
-	**Selection du block location** :
	-	nginx cherche d'abord le prefix location (le path?) le plus spécifique peu importe l'ordre des block. Si il y un seul block avec "/" alors on va le choisir mais s'il en existe un plus précis dans le match c'est celui-ci qui sera selectionné.
	- Lorsque un match est trouvé on utilise toutes les

| Directive         | Action attendue                                |                                                |
| ----------------- | ---------------------------------------------- | ---------------------------------------------- |
| `listen [IP]:[port]`   | Créer un socket sur IP\:port                |
| `server_name`          | Comparer avec `Host:` de la requête HTTP    |
| `error_page code path` | Retourner ce fichier si `code` est retourné |
| `client_max_body_size` | Limiter la taille du body pour POST/PUT     |
| `methods`         | Autoriser seulement les méthodes données       |                                                |
| `return code URL` | Redirection                                    |                                                |
| `root`            | Répertoire associé à cette URL                 |                                                |
| `index`           | Fichier par défaut à retourner dans un dossier |                                                |
| \`autoindex on    | off\`                                          | Si fichier manquant, lister contenu du dossier |
| `cgi_pass path`   | Utiliser ce programme CGI selon extension      |                                                |
| `upload_dir path` | Sauvegarder les uploads ici                    |                                                |

### Méthode GET

*	[Documentation GET](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Methods/GET)

Dans le cas où la méthode est `GET`, le **client veut récupérer des informations à partir d'un ressource spécifiée, sans modifier les données**.

Que faire pour la traiter :
-	Décider quel fichier on doit envoyer, analyser l'uri/path :
	-	Si le `path == "/"` -> on renvoie l'index par défaut, et on compléte le *Header* de réponse en cohérence.
	-	Sinon selon l'extension du fichier renvoyer le type de contenu associé, exemple `path_ext == "jpg" || "JPG"` -> on va envoyer une image, compléter le *Header* en fonction avec "Content-Type: image/jpg\r\n". Vérifier tous les cas possibles css, js, ttf, html etc.
-	**status** : 
	-	`200` (OK) -> la demande a réussie.
	-	`404` (NOT FOUND) -> n'est pas trouvé par le serveur.
	-	`400` (WRONG REQUEST) -> la requete a mal été formatée.
-	Ne doit pas avoir de body, la facon dont on gère l'erreur est indéfini, on peut renvoyer `status 400`.

À tester :
-	Les requetes GET doitvent toujours renvoyer les memes résultats si on les effectue plusieurs fois.

### Méthode POST

Dans le cas où la méthode est `POST` :
-	