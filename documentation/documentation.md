# Webserv Documentation

#### Sommaire des pages de documentation
* [Server HTTP - World Wide Web](general.md)
* [Server Configuration File](configuration.md)
* [Connexion Clien-Server avec Sockets](socket.md)
* [HTTP Request handling](request_http.md)
* [CGI](cgi.md)
* [Processus du serveur - fonctionnement général](processus.md)
* [Epoll](epoll.md)


### Tutoriels, guides et documentation de références

* [Bonnes pratiques en C++](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

**Compréhension globale** :

* [Vidéo - Web Server Concepts and Examples](https://www.youtube.com/watch?v=9J1nJOivdyw) - Bon départ de compréhension bonne illustration du du projet

* [Turning Portal 2 into a Web Server](https://www.youtube.com/watch?v=-v5vCLLsqbA) - vidéo recommandée par le staff de 42 pour comprendre des notions et s'interesser au projet (partagée sur le chan discord de webserv)

* [Tutoriel HTTP Web Server en C++](https://github.com/Dungyichao/http_server) <- ***Très complet***, avec pleins de ressources super utiles

* [42Webserv Projet - Repo Github](https://github.com/Kaydooo/Webserv_42) - peu aider à comprendre ce qui est attendu spécifiquement par le sujet 42 + contient des infos et liens de documentation utiles

* [42WebServ wiki](https://github.com/Tablerase/42_WebServ/wiki/Home/ed1da06241dbecd510ffb702094d7af7fff191bf) - page du projet et ressource, propre au sujet 42

* [Tutoriel pour écrire un miniserveur](https://bousk.developpez.com/cours/reseau-c++/TCP/08-premier-serveur-mini-serveur/) - aide à la compréhension et pour structurer les classes Client et Server

* [Webserv: A C++ Webserver](https://hackmd.io/@laian/SJZHcOsmT) - Résumé des concepts abordés par Webserv

**Gestion réseau - Sockets** :

* [Guide pour programmation en réseau](https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf) - guide très recommandé pour la compréhension des sockets
	* [Traduction - Guide pour la programmation en réseau](http://vidalc.chez.com/lf/socket.html)

* [Building socket server](https://ncona.com/2019/04/building-a-simple-server-with-cpp/) - Explications sur les fonctions de gestion des sockets

* [Socket Programming - GeeksForGeeks](https://www.geeksforgeeks.org/socket-programming-cc/) - Explications sur les fonctions de gestion des sockets

* [Vidéo tuto socket](https://www.youtube.com/watch?v=s3o5tixMFho)

* [Vidéo - Node's Event Loop](https://www.youtube.com/watch?v=P9csgxBgaZ8) - comprendre des notions de gestion de sockets + polling

* [Documentation IBM - Server Network Deployment](https://www.ibm.com/docs/fr/was-nd/8.5.5?topic=icwspi-configuring-web-server-application-server-profile-same-machine) - informations sur tout le processus, peut servir de support d'inspiration et/ou de compréhension

* [Building a Non-Blocking Web Server](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7) - guide pour rendre le serveur non-bloquant

**Gestion HTTP Request** :

* [Understanding Nginx Server and Location Block Selection Algorithms](https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms) - article très recommandé pour comprendre la selection de blocks de Nginx pour traiter une HTTP request

* [Guide/Dictionnaire HTTP](https://developer.mozilla.org/fr/docs/Web/HTTP/Guides/Overview) - source fiable du fonctionnement HTTP

* [Wikipedia HTTP](https://en.wikipedia.org/wiki/HTTP)

**Gestion des CGI** :

* [CGI guide](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm) - guide pour comprendre et implémenter les CGI

