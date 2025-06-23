## Qu'est-ce que le CGI ?

**CGI (Common Gateway Interface)** : interface standardisée qui permet aux serveurs web d'exécuter des programmes externes pour traiter les requetes HTTP des utilisateurs cote client. 

Le *CGI* permet donc un échange de données entre des applications externes et les sevreurs web.

Fonctionnement du CGI :
- **Interaction Utilisateur** : Lorsqu'un utilisateur remplit un formulaire sur une page web et le soumet, les données du formulaire sont envoyées au serveur web via une requete HTTP.
- **Traitement par le serveur** : Le serveur recoit cette requete et lance un programme CGI, souvent ecrit avec un langage de script comme Python.
- **Execution du script CGI** :Le script CGI traite les données recues, effectue les operations necessaires (comme l'accès à une base de données) et génère une réponse.
- **Reponse au client** : Le serveur renvoie la réponse générée par le script CGI u client, généralement sous forme d'une page HTML.

