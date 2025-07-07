# Epoll

## Qu'est ce qu'epoll ? 
C’est une interface Linux permettant de gérer efficacement de multiples descripteurs de fichiers comme des sockets, pipes, etc. C’est utile pour les serveurs, proxies et tout programme qui a besoin de réagir à plusieurs évènements sans bloquer. Il remplace select ou  poll quand tu as beaucoup de connexions simultanées car il est plus scalable et performant. 

#### <ins>Bibliotheques</ins> :
- **<sys/epoll.h>**
- **<fcntl.h>** (pour `fcntl()`, pour rendre un descripteur non bloquant, indispensable pour **epoll**)

## Fcntl()
### int fcntl(int fd, int cmd, arg);
Fonction qui permet de manipuler des descripteurs de fichiers (ici sockets). Elle permet d'obtenir ou modifier les flags d'un descripteur (le rendre non bloquant par exemple).

#### <ins>Arguments</ins> :
- **fd** : le descripteur de fichiers qu'on veut manipuler
- **cmd** : la commande qu'on veut effectuer
	- **F_GETFL** : lire les flags du descripteur (retourne les flags actuels comme O_NONBLOCK, O_APPEN, etc) Chaque bit du int retourne correspond a un flag (0 : aucun flag, O_NONBLOCK : 04000 en octal, O_RDWR | O_NONBLOCK : combinaison binaire des deux, -1 : erreur)
	- **F_SETFL** : modifie les flags (ecrase ceux existants si on ne precise pas)
- **arg** (optionnel) : argument utilise selon la commande, mieux de le mettre a 0 que vide pour la compilation (il ne sera juste pas pris en compte)
	- **O_CLOEXEC** : ferme le fd automatiquement apres `exec()`
	- **O_NONBLOCK** : rend le fd non bloquant (`read()`, `write()`, `acept()` ne bloquent pas)

## Fonctions principales d' epoll :
1. ### int epoll_create1(int flags);
	Fonction qui cree une instance d'epoll (un descripteur de fichier special qui sera utilise dans les autres fonctions).
	#### <ins>Arguments</ins> :
	- **flags** : =0 ou =EPOLL_CLOEXEC (ferme le fd automatiquement apres `exec()`)

2. ### int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	Fonction qui ajoute, modifie ou supprime un descripteur surveille par epoll. Elle retourne 0 en cas de succes et -1 en cas d'erreur (descripteur invalide, fd deja enregistre, quand on essaye de supprimer un fd non enregistre, etc)

	#### <ins>Arguments</ins> :
	- **epfd** : descripteur d'epoll (cree avec `epoll_create1()`)
	- **op** : operation a effectuer (EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL :respectivemen ajouter, modifier, et supprimer)
	- **fd** : le descripteur de fichier a surveiller (ici socket)
	- **event** : un pointeur vers une structure epoll_event qui decrit les evenements a surveiller

	```
	struct epoll_event
	{
		uint32_t events			//les evenements surveilles (ou detectes)
		epoll_data_t data 		//donnees associees (ex : un fd, un pointeur, etc)
	};
	```

	#### <ins>Evemements surveilles utiles</ins> : 
	- EPOLLIN : pret a lire
	- EPOLLOUT : pret a ecrire
	- EPOLLERR : erreur
	- EPOLLHUB : fermeture de la connexion
	- EPOLLET : edge-triggered (notification une seule fois tant qu'il y a du nouveau). Le mode par defaut est « level-triggered » : tant que l’évènement est là, epoll_wait te le redonne encore et encore. EPOLLET permets donc moins de notifications et est donc plus performant mais oblige à être plus rigoureux car il faut être sur de lire jusqu’à épuisement (read jusqu’à ce qu’il retourne EAGAIN)

3. ### int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
	Cette fonction attend que des evenements se produisent. Elle retourne le nombre d'evenements prets ou -1 si erreur.

	#### <ins>Arguments</ins> :
	- **epfd** : descripteur d'epoll
	- **events** : tableau pour recuperer les evenements prets _(<ins>Remarque</ins>: dans `epoll_ctl()` c'est un pointeur vers une structure epoll_event alors qu'ici c'est un tableau de plusieurs structures epoll_event)_
	- **maxevents** : taille maximale du tableau events
	- **timeout** : en ms (0 = non bloquant, -1 = bloque indefiniement jusqu'a un evenement)

