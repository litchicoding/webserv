***Pour l'instant 

## main.cpp

*logique*:
- Si le programme a été lancé avec un argument `config_file` alors on construit un objet de la classe `Server` avec en argument le nom du fichier.
- Si le programme n'a pas de *config file* on construit `Server` selon des parametres par defaut.

- Si la construction s'est bien passé, on lance l'activité du serveur avec `start` et `update`.

## Server Constructors

On itinialise les attributs membres de la classe `Server`
- `_socket_fd` = pour identifier le socket du serveur
- `_serv_addr` = structure avec des datas sur le serveur et les socket (port, type d'addresse IP(IPv4) etc.)

## start()

- `add_fd_to_epoll()` :On utilise `fcntl()` pour rendre la socket non bloquante puis `epoll_create1()` et 
`epoll_ctl()` pour creer une instance d'epoll et lui donner la socket a surveiller
- Puis `bind()`
- Puis `listen()`

Le serveur a maintenant une **socket** sur écoute!

## update()

Boucle qui crée des *scoket* à chaque tour pour le `client` et attend de recevoir une **request**.

- Si le socket du client est accepté (avec `accept()`) alors on peut proceder au traitement de la requete, puis y repondre.

## stop()

A pour role de fermer le socket du serveur, ce qui interompt la connexion.

- Affiche un message d'erreur si besoin
- `close()` on ferme le socket
- On met à jour la valeur du `socket_fd` pour signifier qu'il est bien fermé et n'est plus utilisable.