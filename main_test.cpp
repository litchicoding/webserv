#include <iostream>
#include <sstream>

#include "inc/Client.hpp" // main pour test a supprimer plus tard !

// g++ -std=c++98 -Wall -Wextra -Werror main_test.cpp src/Client.cpp

int main()
{
	int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_serveur < 0)
	{
		std::cerr << "Error : socket()" << std::endl;
		return 1;
	}

	struct sockaddr_in  server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // = INADDR_ANY = 0

	if (bind(socket_serveur, (struct sockaddr*)&server_addr, sizeof(server_addr)) != SUCCESS)
	{
		std::cerr << "Error : bind()" << std::endl;
		return 1;
	}
	if (listen(socket_serveur, 5) != SUCCESS)
	{
		std::cerr << "Error : listen()" << std::endl;
		return 1;
	}
	std::cout << "🟢 Serveur en écoute sur le port 8080..." << std::endl;
	while (true)
	{
		struct sockaddr_in	client_addr;
		socklen_t			client_len = sizeof(client_addr);
		int client_fd = accept(socket_serveur, (sockaddr *)&client_addr, &client_len);
		
		Client* client = new Client(client_fd, client_addr);
		client->start();
		//vector en dessous !!!
		// clients[client_fd] = client;
	}

	close(socket_serveur);
   	std::cout << "🛑 Connexion fermée." << std::endl;
	return 0;
}
