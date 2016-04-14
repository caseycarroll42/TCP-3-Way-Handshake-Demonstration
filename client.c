#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


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
    short int cksum;
    short int ptr;
    int opt;
};

//bit flags for hdr_flags
enum {
  SYN = 0x01,
  ACK = 0x02,
  FIN = 0x04,
};

int connect_to_server(int portno);
unsigned int compute_cksum(unsigned short int * cksum_arr); //computes checksum
int send_conn_request(uint32_t source_port, int portno, int sockfd); //Create a connection request TCP segment
int send_ack_tcp_seg(int sockfd);

int main(int argc, char **argv)
{
	int portno, sockfd;
  socklen_t local_addr_len;
	int num_attempts = 0;	
  struct sockaddr_in local_addr;
	struct hostnet *server;  
  struct tcp_hdr conn_grant;    

	//check if user passed port number as argument
	if (argc < 2) {
		printf("Usage %s Port_Number \n", argv[0]);
		exit(0);
	}

  portno = atoi(argv[1]);	//set port number from user input
  sockfd = connect_to_server(portno); //connect to server

  /* get source port */
  local_addr_len = sizeof(local_addr);
  getsockname(sockfd, (struct sockaddr *)&local_addr, &local_addr_len);  

  //send the connection request
  send_conn_request(htons(local_addr.sin_port), portno, sockfd);

  //receive response and send acknowledgement tcp segment
  send_ack_tcp_seg(sockfd);  
  
	return 0;
}

int connect_to_server(int portno)
{
  struct sockaddr_in serv_addr;
  int sockfd;
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
  return sockfd;
}
/*
 * Code taken from example on blackboard
 */
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
  return (0xFFFF^cksum);
}

/*
SEND CONNECTION REQUEST
  i. Assign an initial client sequence number with a zero acknowledgement
  number
  ii. Set the SYN bit to 1
  iii. Compute the 16-bit checksum of the entire TCP segment and populate the checksum field
*/

int send_conn_request(uint32_t source_port, int portno, int sockfd)
{
  int num_sent;
  struct tcp_hdr tcp_seg;
  unsigned short int cksum_arr[12];
  char tcp_char_segment[255];
  
  /*set up connection request tcp struct*/
  tcp_seg.src = source_port; //set source port
  tcp_seg.des = portno; //set destination port
  tcp_seg.seq = 0; //Assign an initial client sequence number with 
  tcp_seg.ack = 0; //a zero acknowledgement number
  tcp_seg.hdr_flags |= SYN; //set the SYN bit to 1
  tcp_seg.rec = 0;
  tcp_seg.cksum = 0; 
  tcp_seg.ptr = 0;
  tcp_seg.opt = 0;

  memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes
  tcp_seg.cksum = compute_cksum(cksum_arr); //compute checksum

  /* send TCP struct to server */
  memcpy(tcp_char_segment, &tcp_seg, sizeof(tcp_seg)); //copy struct to char array
  num_sent = write(sockfd, tcp_char_segment, 255); //write to socket
  if (num_sent < 0)
  {
    printf("error writing to socket...\n");
    exit(1);
  }
  return 0; 
}

int send_ack_tcp_seg(int sockfd)
{
  int num_recv, num_sent;
  char tcp_char_seg[255];
  unsigned short int cksum_arr[12];
  struct tcp_hdr conn_grant_seg;
  
  /* read in connection granted segment */
  num_recv = read(sockfd, tcp_char_seg, 255); //read from socket
  if(num_recv < 0)
  {
    printf("error reading from socket...\n");
    exit(1);
  }

  memcpy(&conn_grant_seg, tcp_char_seg, sizeof conn_grant_seg); //copy buffer to segment

  //Assign a sequence number as initial client sequence number + 1 with an acknowledgement number equal to initial server sequence number + 1
  conn_grant_seg.seq += 1;
  conn_grant_seg.ack += 1;
  
  //Set the ACK bit to 1
  conn_grant_seg.hdr_flags |= ACK;
  
  //Compute the 16-bit checksum of the entire TCP segment and populate the checksum field
  memcpy(cksum_arr, &conn_grant_seg, 24);
  conn_grant_seg.cksum = compute_cksum(cksum_arr);

  //respond to server with acknowledgent TCP segment
  bzero(tcp_char_seg,255);
  memcpy(tcp_char_seg, &conn_grant_seg, sizeof conn_grant_seg);
  
  num_sent = write(sockfd, tcp_char_seg, 255); //write back to server
  if (num_sent < 0)
  {
    printf("error writing to socket...\n");
    exit(1);
  }
  return 0;
}