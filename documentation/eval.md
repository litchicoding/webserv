
## PARSING FICHIER DE CONFIG
### Test avec configuration valide
```bash
./webserv ./config/webserv.conf
./webserv
```

### Test avec configuration invalide
#### Attendu : Erreur de parsing
#### ERREUR : Ouvre le .conf par default au lieu de celui indiquer que le fichier soit valide ou non.
```bash
echo "invalid.conf > invalid.conf
./webserv invalid.conf
./webserv invalid.confe
./webserv invalid
```

### Test avec fichier inexistant
#### Attendu : Erreur fichier non trouvé
```bash
./webserv nonexistent.conf
```

### Test parsing
- bloc location manquant = doit en avoir un par défaut
- aucun port configuré
- erreur de syntaxe

## MULTIPORTS ET SERVEURS CONFIG
### Test multiports
```bash
curl -I http://localhost:8080/
curl -I http://localhost:8081/
curl -I http://localhost:8082/
```
### Test plusieurs connexions en meme temps
```bash
# Plusieurs culr en background avec &
curl -X GET http://localhost:8080/ & 
curl -X GET http://localhost:8080/ & 
curl -X POST -F "name=yannick" -F "message=hello" http://localhost:8080/data &
wait

# Envoyer N request en parallèle
seq 20 | xargs -n1 -P10 curl -s http://localhost:8080/
# seq 20 : 20 jobs
# -P10 : 10 request
# -n1 : exécute curl 1x par job
```

### Test différents directives par block serveur
lancer et voir si respect des règles

## REQUETES HTTP BASIQUES
### Headers multiples
```bash
curl -v -H "User-Agent: TestClient/1.0" -H "Accept: text/html" http://localhost:8080
```
Est-ce que si Accept = text/html, si j'envoie du css doit etre une erreur ?

### Requete mal formatée
test classique mais avec erreur (syntaxe, contexte etc)
-	Content-Length incohérent = 400
-	Pas de Content-Length = maybe 411
-	Content-Length + Transfer-Encoding
-	Content-Type pas supporté
-	Header mal formaté : "Content-Length 100"

### Methode pas supportée
```bash
curl -X PATCH http://localhost:8080/
```

### Version invalide
```bash
echo -e "GET / HTTP/2.0\r\n\r\n" | nc localhost 8080
```

## HEADER REQUIS HTTP/1.1

### Headers manquants
retirer un Header obligatoire dans un certain contexte
```bash
echo -e "GET / HTTP/1.1\r\n\r\n" | nc localhost 8080
```

### Header requis
#### Attendu : Content-Type, Content-Length, Last-Modified
```bash
curl -I http://localhost:8080/test.txt
```

## DOWNLOAD

### Fichier texte
```bash
curl -v -o downloaded_test.txt http://localhost:8080/data/test.txt
diff ./data/test.txt downloaded_test.txt
```
### Fichier binaire
test avec images ou "file.bin"

### RANGE REQUEST
### Premier kilobyte
#### Attendu : HTTP/1.1 206 Partial Content
curl -v -H "Range: bytes=0-1023" http://localhost:8080/large_file.bin

### Depuis position 1024 jusqu'à la fin
#### Attendu : HTTP/1.1 206 Partial Content
curl -v -H "Range: bytes=1024-" http://localhost:8080/large_file.bin

### Range invalide
#### Attendu : HTTP/1.1 416 Range Not Satisfiable
curl -v -H "Range: bytes=999999999-" http://localhost:8080/test.txt


## UPLOAD

### POST simple
#### Attendu : 201 Created + Location
```bash
curl -v -X POST -d "test data" http://localhost:8080/upload
```

### POST avec Content-Type
```bash
curl -v -X POST -H "Content-Type: text/plain" -d "test content" http://localhost:8080/upload
```
-	Sans Content-Type
-	Sans Boundary
-	Pas de Content-Length = maybe 411

### POST fichier
```bash
curl -v -X POST -F "file=@test.txt" http://localhost:8080/upload
curl -v -X POST -F "file=@/images/image_1.jpg" http://localhost:8080/upload
```

### Chunked Trasnfer Encoding
#### Attendu : 201 Created ou 200
```bash
curl -v -X POST http://localhost:8888/upload \
     -H "Transfer-Encoding: chunked" \
     -d "Hello World"

cat << 'EOF' > chunked_request.txt
POST /upload HTTP/1.1
Host: localhost:8080
Transfer-Encoding: chunked
\r\n
4
test
5
hello
0
\r\n
EOF

cat chunked_request.txt | nc localhost 8080
```

### Test application/x-www-form-urlencoded
```bash
curl -v -X POST \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=John&email=john@example.com&message=Hello" \
  http://localhost:8080/form-handler
```

### Test multipart/form-data
```bash
curl -v -X POST \
  -F "name=John" \
  -F "file=@test.txt" \
  -F "description=Test file upload" \
  http://localhost:8080/upload

#http://localhost:8080/multipart-handler
```

## DELETE

### Supprimer un fichier
```bash
curl -v -X DELETE http://localhost:8080/file.txt -> 405 method not allowed
curl -v -X DELETE http://localhost:8080/upload/file.txt -> 204 no content
```

### Supprimer un fichier inexistant
```bash
curl -v -X DELETE http://localhost:8080/upload/nonexistent.txt -> 404 not found
```

### Supprimer ressource protégée
enlever des droits sur un fichier ou dossier, pas DELETE autorisée dans tel dossier etc

```bash
chmod 000 file.txt puis curl -v -X DELETE http://localhost:8080/upload/file.txt --> 403 forbidden
```

## REDIRECTIONS

### Redirection permanente (301)
#### Attendu : HTTP/1.1 301 Moved Permanently + Location header
```bash
curl -v http://localhost:8080/redirect
```

### Suivre les redirections de la config
#### Attendu : Redirection suivie automatiquement + Location header
```bash
curl -v -L http://localhost:8080/redirect
```

### Client qui essayent de supprimer le meme fichier en meme temps
#### Attendu : 404 si fichier déjà supprimé

```bash
curl -v -X DELETE http://localhost:8080/upload/file.txt & curl -v -X DELETE http://localhost:8080/upload/file.txt -> 404 not found pour le deuxieme 
```

## ERROR PAGES

### Page 404, 500, 403, 413 custom
```bash
curl -v http://localhost:8080/nonexistent-page -> 404 not found
curl -v http://localhost:8080/not-allowed-page -> 403 forbidden
```

## LISTING 

### Autoindex on
#### Attendu : Liste des fichiers presents
```bash
curl -v http://localhost:8080/pages/ -> OK
```

### Autoindex off
#### Attendu : 403 Forbidden
```bash
curl -v http://localhost:8080/pages/ -> 403 access forbidden
```

## CGI

### Executer le script
#### Attendu : HTML généré par le script
```bash
curl -v http://localhost:8080/cgi-bin/form.php -> 200 OK
curl -v http://localhost:8080/cgi-bin/hello.py -> 200 OK
```

### CGI avec paramètres GET
```bash
curl -v "http://localhost:8080/test.cgi?param1=value1&param2=value2"
```

### CGI avec POST
```bash
curl -v -X POST -d "input=test" http://localhost:8080/test.cgi
```

## PERFORMANCES ET NON-BLOQUANT

### Connexion simultanées
```bash
# Installer siege si nécessaire : apt-get install siege

# Test de charge basique
siege -c 10 -t 30s http://localhost:8080/

# Test avec fichiers de différentes tailles
siege -c 50 -t 60s -f urls.txt
# urls.txt contient :
# http://localhost:8080/
# http://localhost:8080/test.txt
# http://localhost:8080/large_file.bin
```

### Connection Keep-Alive
```bash
curl -v -H "Connection: keep-alive" http://localhost:8080/ http://localhost:8080/data/test.txt

# Test avec ab (Apache Bench)
ab -n 1000 -c 10 -k http://localhost:8080/
```

### Timeout connection
```bash
# Connexion sans envoyer de données
# Attendu : Attendre le timeout (devrait fermer automatiquement)
telnet localhost 8080

# Connexion lente (slow loris simulation)
python3 << 'EOF'
import socket
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8080))
s.send(b'GET / HTTP/1.1\r\nHost: localhost\r\n')
time.sleep(30)  # Attendre 30 secondes
s.send(b'\r\n')
print(s.recv(1024).decode())
s.close()
EOF
```

### Connection close
#### Attendu : vérifier que la connexion se ferme après la réponse
```bash
curl -v -H "Connection: close" http://localhost:8080/
```

### Client qui ne termine jamais sa requete
#### Attendu : le serveur ne doit pas bloquer les autres clients et finir par fermer la connexion après un timeout
Trouver la commande

## HTTP/1.1 VALIDITY

### Header trop longs
#### Attendu : 431 Request Header Fields Too Large
```bash
curl -v -H "X-Very-Long-Header: $(python3 -c 'print("A"*10000)')" http://localhost:8080/
```

### Body trop large
#### Attendu : 413 Playload too large
```bash
dd if=/dev/zero bs=1M count=100 | curl -v -X POST -T - http://localhost:8080/upload
```

### Requête avec caractères nulls
#### Attendu : 400 Bad Request
```bash
echo -e "GET /\x00test HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080
```

## COMPARE WITH NGINX
```bash
diff <(curl -s -I http://localhost:8080/) <(curl -s -I http://nginx-server/)
```

## Script automatisé
```bash
#!/bin/bash
# test_suite.sh

echo "=== HTTP/1.1 Server Test Suite ==="

# Configuration
SERVER_URL="http://localhost:8080"
NGINX_URL="http://localhost:8081"  # Pour comparaison

# Fonction de test
run_test() {
    echo -n "Testing $1... "
    if eval "$2" >/dev/null 2>&1; then
        echo "✓ PASS"
    else
        echo "✗ FAIL"
        echo "Command: $2"
    fi
}

# Tests basiques
run_test "Basic GET" "curl -s -f $SERVER_URL/"
run_test "404 Error" "curl -s -w '%{http_code}' $SERVER_URL/nonexistent | grep 404"
run_test "File download" "curl -s -f $SERVER_URL/test.txt -o /tmp/downloaded"
run_test "POST data" "curl -s -f -X POST -d 'test' $SERVER_URL/upload"
run_test "Directory listing" "curl -s -f $SERVER_URL/listing/"

echo "=== Comparison with Nginx ==="
echo "Your server response:"
curl -I $SERVER_URL/ 2>/dev/null | head -1
echo "Nginx response:"
curl -I $NGINX_URL/ 2>/dev/null | head -1

echo "=== Performance Test ==="
echo "Running siege test..."
siege -c 10 -t 10s -q $SERVER_URL/
```

- launch the installation of siege with homebrew
- Explain the basics of an HTTP server.
- Ask which function they used for I/O Multiplexing.
- Ask to get an explanation of how select (or equivalent) is working.
- Ask if they use only one select (or equivalent) and how they've managed the server accept and the client
read/write.
- **the select (or equivalent) should be in the main loop and should check fd for read and write AT THE SAME TIME**, if not please give a 0 and stop the evaluation.
- **There should be only one read or one write per client per select** (or equivalent). Ask to show you the code that goes from the select (or equivalent) to the read and write of a client.
- Search for all read/recv/write/send on a socket and check that if **an error returned the client is removed**
- Search for all read/recv/write/send and check if the returned value is well checked. (checking only -1 or 0 is not good you should check both)
- **If a check of errno is done after read/recv/write/send. Please stop the evaluation and put a mark to 0**
- **Writing or reading ANY file descriptor** without going through the select (or equivalent) is strictly FORBIDDEN
- The project must compile without any re-link issue if not use the compile flag.
- If any point is unclear or is not correct use the flag for incomplete work

In the configuration file check if you can do the following and test the result:
- look for the HTTP response status codes list on internet and during this evaluation
if any status codes is wrong don't give related points.
- setup multiple servers with different port
- setup multiple servers with different hostname (use something like: curl --resolve example.com:80:127.0.0.1
http://example.com/)
- setup default error page (try to change the error 404)
- limit the client body (use curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- setup routes in a server to different directories
- setup a default file to search for if you ask for a directory
- setup a list of method accepted for a certain route (ex: try to delete something with and without permission)
- TEST PARSING


Using telnet, curl, prepared files demonstrates that the following features work properly:
- GET requests -> should work
- POST requests -> should work
- DELETE requests -> should work
- UNKNOWN requests -> should not produce any crash
- For every test the status code must be good
- upload some file to the server and **get it back**
- Use the reference browser of the team, open the network part of it and try to connect to the server with it
- Look at the request header and response header
- It should be compatible to serve a fully static website
- Try a wrong URL on the server
- Try to list a directory
- Try a redirected URL
- Try things

- In the configuration file setup multiple ports and use different websites, use the browser to check that the
configuration is working as expected, and show the right website.
- In the configuration try to setup the same port multiple times. It should not work.
- Launch multiple servers at the same time with different configurations but with common ports. Is it working? If it is
working, ask why the server should work if one of the configurations isn't working. keep going

- Use Siege to run some stress tests.
- Availability should be above 99.5% for a simple get on an empty page with a siege -b on that page
- Check if there is no memory leak (monitor the process memory usage it should not go up indefinitely)
- Check if there is no hanging connection
- You should be able to use siege indefinitely without restarting the server (look at siege -b)
