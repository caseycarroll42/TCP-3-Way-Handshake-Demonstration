#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT_NO 6664
#define SERVER_SEQ 200

//initialize TCP header struct
struct tcp_hdr
{
    short int src;
    short int des;
    int seq;
    int ack;
    unsigned char tcph_reserved:4, tcph_offset:4;
    short int hdr_flags;
    short int rec;
     int cksum;
    short int ptr;
    int opt;
};

//bit flags for hdr_flags
enum {
	SYN = 0x01,
	ACK = 0x02,
	FIN = 0x04,
};

int server_socket();
int connect_to_client(int serv_sock);
unsigned int compute_cksum(unsigned short int * cksum_arr); //computes checksum
int recv_conn_req(int accept_sock);
int recv_ack_seg(int accept_sock);
int recv_close_req(int accept_sock);
void print_tcp_seg(struct tcp_hdr *tcp_seg);

int main(int argc, char **argv)
{
	char buffer[255];
	int num_data_recv, num_sent;	  	
	struct tcp_hdr tcp_seg;
	struct tcp_hdr tcp_ack_seg;

	//clear output file
	FILE *fp;

	fp = fopen ("server_output.txt", "w");
	fclose(fp);

	int serv_sock = server_socket();
	int accept_sock = connect_to_client(serv_sock);

	printf("\t\tPART 1\n");
	//handle the connection request
	recv_conn_req(accept_sock);
	//handle the acknowledgement segment from client
	recv_ack_seg(accept_sock);

	printf("\t\tPART 2\n");
	//handle the close connection request from client
	recv_close_req(accept_sock);

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

unsigned int compute_cksum(unsigned short int * cksum_arr)
{  
  unsigned int i,sum=0, cksum;
  
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
  return (cksum);
}

//The server responds to the request by creating a connection granted TCP segment. 
int recv_conn_req(int accept_sock)
{
	int num_data_recv, num_sent, temp_portno;
	char buffer[255];
	struct tcp_hdr tcp_seg;
	unsigned short int cksum_arr[12];

	num_data_recv = read(accept_sock, buffer, 255);
	if(num_data_recv < 0)
	{
		printf("error receiving from socket\n");
		exit(1);
	}

	memcpy(&tcp_seg, buffer, sizeof(tcp_seg));

	printf("-----CONNECTION REQUEST SEGMENT FROM CLIENT-----\n");
	print_tcp_seg(&tcp_seg);	

	//set SYN bit to and ACK bit to 1
	tcp_seg.hdr_flags = (SYN | ACK);

	//Assign an initial server sequence number with an acknowledgement number equal to initial client sequence number + 1
	tcp_seg.ack = tcp_seg.seq + 1; //101
	tcp_seg.seq = SERVER_SEQ; //200
	
	temp_portno = tcp_seg.src;
	tcp_seg.src = tcp_seg.des;
	tcp_seg.des = temp_portno; //change the source and destination to go back to client
	
	/* compute checksum */
	memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes
  	tcp_seg.cksum = compute_cksum(cksum_arr); //compute checksum  	
  	printf("0x%04X\n", (0xFFFF^tcp_seg.cksum));

  	printf("-----CONNECTION ACCEPTED SEGMENT TO CLIENT-----\n");
  	print_tcp_seg(&tcp_seg);

	/* send connection granted segment to client */
	memcpy(buffer, &tcp_seg, sizeof tcp_seg);	//copy segment to char buffer
	num_sent = write(accept_sock, buffer, 255); //send buffer to client
	if (num_sent < 0)
	{
	  printf("error writing to socket...\n");
	  exit(1);
	}	
	return 0;
}

//receive the acknowledgement segment from the client
int recv_ack_seg(int accept_sock)
{
	int num_data_recv;
	char buffer[255];
	struct tcp_hdr tcp_ack_seg;

	/* receive acknowledgement TCP segment from client */
	num_data_recv = read(accept_sock, buffer, 255);
	if (num_data_recv < 0)
	{
		printf("error receiving data from socket\n");
		exit(1);
	}

	memcpy(&tcp_ack_seg, buffer, sizeof tcp_ack_seg);

	printf("-----RECEIVED ACKNOWLEDGEMENT SEGMENT-----\n");
	print_tcp_seg(&tcp_ack_seg);
	return 0;
}

int recv_close_req(int accept_sock)
{
	int num_data_recv, num_sent, temp_portno;
	char buffer[255];
	struct tcp_hdr tcp_close_seg;
	unsigned short int cksum_arr[12];

	/* receive acknowledgement TCP segment from client */
	num_data_recv = read(accept_sock, buffer, 255);
	if (num_data_recv < 0)
	{
		printf("error receiving data from socket\n");
		exit(1);
	}

	memcpy(&tcp_close_seg, buffer, sizeof tcp_close_seg);

	printf("-----RECEIVED CLOSE REQUEST SEGMENT-----\n");
	print_tcp_seg(&tcp_close_seg);	

	temp_portno = tcp_close_seg.src;
	tcp_close_seg.src = tcp_close_seg.des;
	tcp_close_seg.des = temp_portno; //change the source and destination to go back to client

	tcp_close_seg.ack = tcp_close_seg.seq + 1;
	tcp_close_seg.hdr_flags = ACK;

	memcpy(cksum_arr, &tcp_close_seg, 24); //Copying 24 bytes
	tcp_close_seg.cksum = compute_cksum(cksum_arr);

	printf("-----TRANSMITTED CLOSE ACKNOWLEDGEMENT-----\n");
	print_tcp_seg(&tcp_close_seg);

	memcpy(buffer, &tcp_close_seg, sizeof tcp_close_seg);	//copy segment to char buffer
	num_sent = write(accept_sock, buffer, 255); //send buffer to client
	if (num_sent < 0)
	{
	  printf("error writing to socket...\n");
	  exit(1);
	}		

	tcp_close_seg.hdr_flags = FIN;
	tcp_close_seg.seq = SERVER_SEQ;

	memcpy(cksum_arr, &tcp_close_seg, 24); //Copying 24 bytes
	tcp_close_seg.cksum = compute_cksum(cksum_arr);

	printf("-----TRANSMITTED SECOND CLOSE ACKNOWLEDGEMENT-----\n");
	print_tcp_seg(&tcp_close_seg);

	bzero(buffer,255);
	memcpy(buffer, &tcp_close_seg, sizeof tcp_close_seg);	//copy segment to char buffer
	num_sent = write(accept_sock, buffer, 255); //send buffer to client
	if (num_sent < 0)
	{
	  printf("error writing to socket...\n");
	  exit(1);
	}

	return 0;
}

void print_tcp_seg(struct tcp_hdr *tcp_seg)
{
	FILE *fp;

	fp = fopen("server_output.txt", "a+");

	/*Print out tcp connection request */
	printf("source port:\t\t%hu\n", tcp_seg->src);
	printf("destination:\t\t%hu\n", tcp_seg->des);
	printf("sequence:\t\t%d\n", tcp_seg->seq);
	printf("acknowledgement:\t%d\n", tcp_seg->ack);
	
	if(tcp_seg->hdr_flags & SYN)
	{
		printf("hdr_flags: SYN = 1\n");
	} if (tcp_seg->hdr_flags & ACK) 
	{
		printf("hdr_flags: ACK = 1\n");
	} if (tcp_seg->hdr_flags & FIN)
	{
		printf("hdr_flags: FIN = 1\n");
	}

	printf("hdr flags:\t\t0x0%x\n", tcp_seg->hdr_flags);
	printf("receive window:\t\t%hu\n", tcp_seg->rec);
	printf("checksum:\t\t0x%X\n", (0xFFFF^tcp_seg->cksum));
	printf("data pointer:\t\t%hu\n", tcp_seg->ptr);
	printf("options:\t\t%d\n", tcp_seg->opt);
	printf("-----------\n\n");

	  /* Print to file */
	fprintf(fp, "source port:\t\t%hu\n", tcp_seg->src);
  	fprintf(fp,"destination:\t\t%hu\n", tcp_seg->des);
  	fprintf(fp,"sequence:\t\t%d\n", tcp_seg->seq);
  	fprintf(fp,"acknowledgement:\t%d\n", tcp_seg->ack);
  
  	if(tcp_seg->hdr_flags & SYN)
  	{
    	fprintf(fp,"hdr_flags: SYN = 1\n");
  	} if (tcp_seg->hdr_flags & ACK) 
  	{
    	fprintf(fp,"hdr_flags: ACK = 1\n");
  	} if (tcp_seg->hdr_flags & FIN)
  	{
    	fprintf(fp,"hdr_flags: FIN = 1\n");
  	}

  	fprintf(fp,"hdr flag actual value:\t\t0x0%x\n", tcp_seg->hdr_flags);
  	fprintf(fp,"receive window:\t\t%hu\n", tcp_seg->rec);
  	fprintf(fp,"checksum:\t\t0x%04X\n", (0xFFFF^tcp_seg->cksum));
  	fprintf(fp,"data pointer:\t\t%hu\n", tcp_seg->ptr);
  	fprintf(fp,"options:\t\t%d\n", tcp_seg->opt);
  	fprintf(fp,"-----------\n\n");

  	fclose(fp);

	return;
}