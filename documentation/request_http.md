# Anatomie d'une requete HTTP

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

### Etapes du processus HTTP

***The client enters the *URL* and the *browser* constructs an HTTP request.***

Lorsqu'un client souhaite communiquer avec le serveur, voici les différentes étapes :
-	1. **Opening a TCP connection:** la connection doit etre établie pour transmettre/recevoir les requetes/réponses depuis le serveur. 
-	2. **Send an HTTP request:** Le client envoie une requete HTTP à travers la connexion *TCP* suivant le bon format de requete.
-	3. **Receive an HTTP response:** Le serveur interprete la requete HTTP et prepare une reponse, puis l'envoie selon le bon format.
-	4. **Close the TCP connection:**