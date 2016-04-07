#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT_NO 666420

int server_socket();
int connect_to_client(int serv_sock);

int main(int argc, char **argv)
{
	int serv_sock = server_socket();
	int accept_sock = connect_to_client();
	
	return 0;
}


/******************************************************
 *Create a socket and bind it to a port so that it can 
 * listen for incoming connections.
 * Returns the socket id 
******************************************************/
int server_socket() 
{
	int serv_sock, clilen;
	
	struct sockaddr_in serv_addr;
	

	//create socket
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(serv_sock < 0)
	{
		printf("Socket could not be opened...\n");
		exit(1);
	}

	//initialize socket structure
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_NO);

	//bind the socket to the port
	if(bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("error while binding...\n");
		exit(1);
	}

	return(serv_sock);
}

/******************************************************
 * Connect to a client with the socket made in create_server().
 * This function listens for a connection from the client
 * The function will accept the connection and return the socket id for this connection
 * Parameters: the socket connected to the client
******************************************************/
int connect_to_client(int serv_sock)
{
	struct sockaddr_in cli_addr;
	socklen_t addr_size;

	int accept_sock, num_data_recv;
	char buffer[256];

	listen(serv_sock, 5);
	printf("listening on port %d\n", PORT_NO);

	addr_size = sizeof(cli_addr);

	accept_sock = accept(serv_sock, (struct sockaddr *)&cli_addr, &addr_size);
	if(accept_sock < 0)
	{
		printf("error accepting\n");
		exit(1);
	}

	return accept_sock;
}