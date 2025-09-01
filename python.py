import socket

HOST = "127.0.0.1"
PORT = 8888

chunks = ["Hello ", "World ", "this ", "is ", "a ", "chunked ", "request!"]

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# Construire toute la requête d'un coup
request = "POST /upload HTTP/1.1\r\n"
request += "Host: localhost\r\n"
request += "Transfer-Encoding: chunked\r\n"
request += "Content-Type: text/plain\r\n\r\n"

# Ajouter tous les chunks
for chunk in chunks:
    size = hex(len(chunk))[2:]
    request += f"{size}\r\n{chunk}\r\n"

# Chunk final
request += "0\r\n\r\n"

# Envoyer tout d'un coup
s.send(request.encode())

# Recevoir la réponse
response = s.recv(4096)
print(response.decode())
s.close()