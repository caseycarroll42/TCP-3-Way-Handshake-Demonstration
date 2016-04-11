#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


//initialize TCP header struct
struct tcp_hdr
{
    short int src;
    short int des;
    int seq;
    int ack;
    short int hdr_flags;
    short int rec;
    short int cksum;
    short int ptr;
    int opt;
};

int connect_to_server(int portno);

int main(int argc, char **argv)
{
	int portno;
	int num_attempts = 0;
	struct sockaddr_in serv_addr;
	struct hostnet *server;


	//check if user passed port number as argument
	if (argc < 2) {
		printf("Usage %s Port_Number \n", argv[0]);
		exit(0);
	}

	//set port number from user input
	portno = atoi(argv[1]);	

	//attempts to connect to server three times
	while(num_attempts < 2)
	{
		if(connect_to_server(portno) == 0)
		{
			//successful connection, break out of loop
			num_attempts = 2;
		} else {
			printf("attempting to connect again...\n");
		}

		num_attempts++;
	}

	return 0;
}

int connect_to_server(int portno)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostnet *server;
   	
   	//create socket
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   	if(sockfd < 0 )
   	{
   		printf("Error creating socket\n");
   		return(-1);
   	}

   	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr=inet_addr("129.120.151.94"); //IP for cse01.cse.unt.edu
   	serv_addr.sin_port = htons(portno);

   	/* Connecting to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("ERROR while attempting to  connect");
		return(-1);
	}

	return 0;//success
}