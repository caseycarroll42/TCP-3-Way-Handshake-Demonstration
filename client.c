#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char **argv)
{
	int portno, sockfd;

	//check if user passed port number as argument
	if (argc < 2) {
		printf("Usage %s Port_Number \n", argv[0]);
		exit(0);
	}

   	//set port number from user input
   	portno = atoi(argv[1]);

   	//create socket
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   	if(sockfd < 0 )
   	{
   		printf("Error creating socket\n");
   		exit(1);
   	}

   	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr("129.120.151.94"); //IP for cse01.cse.unt.edu
   	serv_addr.sin_port = htons(portno);

	return 0;
}