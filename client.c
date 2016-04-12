#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>


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

int main(int argc, char **argv)
{
	int portno, sockfd, local_addr_len, num_sent;
	int num_attempts = 0;
	struct sockaddr_in serv_addr;
  struct sockaddr_in local_addr;
	struct hostnet *server;
  struct tcp_hdr tcp_seg;
  unsigned short int cksum_arr[12];
  unsigned int i,sum=0, cksum;
  char * tcp_char_segment;

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

   	/* Connecting to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("ERROR while attempting to  connect");
		exit(1);
	}  

  //get source port
  local_addr_len = sizeof(local_addr);
  getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len);
  printf("%d\n", local_addr.sin_port);

  //calculate checksum
  tcp_seg.src = local_addr.sin_port;
  tcp_seg.des = portno;
  tcp_seg.seq = 1;
  tcp_seg.ack = 2;
  tcp_seg.hdr_flags = 0x2333;
  tcp_seg.rec = 0;
  tcp_seg.cksum = 0;
  tcp_seg.ptr = 0;
  tcp_seg.opt = 0;

  memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes
  
  printf("0x%04X\n", tcp_seg.src); // Printing all values
  printf("0x%04X\n", tcp_seg.des);
  printf("0x%08X\n", tcp_seg.seq);
  printf("0x%08X\n", tcp_seg.ack);
  printf("0x%04X\n", tcp_seg.hdr_flags);
  printf("0x%04X\n", tcp_seg.rec);
  printf("0x%04X\n", tcp_seg.cksum);
  printf("0x%04X\n", tcp_seg.ptr);
  printf("0x%08X\n", tcp_seg.opt);
 
  for (i=0;i<12;i++)               // Compute sum
   sum = sum + cksum_arr[i];

  cksum = sum >> 16;              // Fold once
  sum = sum & 0x0000FFFF; 
  sum = cksum + sum;

  cksum = sum >> 16;             // Fold once more
  sum = sum & 0x0000FFFF;
  cksum = cksum + sum;

  /* XOR the sum for checksum */
  printf("Checksum Value: 0x%04X\n", (0xFFFF^cksum)); //print result
  tcp_seg.cksum = (0xFFFF^cksum); //assign result to cksum value in struct

  /* copy the tcp_seg to a char array to send to server*/
  tcp_char_segment = (char *)malloc(sizeof(tcp_seg)); //dynamically allocate char array
  memcpy(tcp_char_segment, &tcp_seg, sizeof(tcp_seg)); //copy struct to char array

  /* send char array to server */
  num_sent = write(sockfd, tcp_char_segment, strlen(tcp_char_segment));
  if (num_sent < 0)
  {
    printf("error writing to socket...\n");
    exit(1);
  }

	return 0;
}