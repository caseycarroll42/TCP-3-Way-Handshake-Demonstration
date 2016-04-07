#include <stdio.h>
#include <string.h>



struct tcp_hdr{
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
                
                 
int main(void)
{
 
  struct tcp_hdr tcp_seg;
  unsigned short int cksum_arr[12];
  unsigned int i,sum=0, cksum;

  tcp_seg.src = 21334;
  tcp_seg.des = 2200;
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
  printf("Checksum Value: 0x%04X\n", (0xFFFF^cksum));

}
  