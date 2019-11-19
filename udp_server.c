/**
 * @author Jennifer Norby
 * @version 11/5/2019
 * @reference Duo Lu udp_server.c
 * udp_server.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

/*
 * Definition of constants
 */
#define MAGIC_1 0x4A // ascii for J
#define MAGIC_2 0x4E // ascii for N
#define OPCODE_POST 0x01 // post
#define OPCODE_PACK 0x02 // post ack
#define OPCODE_RETR 0x03 // retrieve
#define OPCODE_RACK 0x04 // retrieve ack

/*
 * Start of main function
 */
int main() {
	FILE *fp;
	char buff[255];

	// an initilization of the file, creates it if it doesn't exist
	fp = fopen("/tmp/test.txt", "w+");
	fgets(buff, 255, (FILE*)fp);
	fclose(fp);
	
	char opcode;
	int ret;
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	char recv_buffer[1024];
	char send_buffer[1024];
	char retrv_msg[201];
	char *retstr;
	int recv_len;
	char recent_msg[201];
	socklen_t len;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("socket() error: %s.\n", strerror(errno));
		return -1;
	}

	// The servaddr is the address and port number that the server will 
	// keep receiving from.
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(32000);

	bind(sockfd, 
		(struct sockaddr *) &servaddr, 
		sizeof(servaddr));

	while (1) {
		fflush(fp);
		int m = 0;	// initialization of m
		len = sizeof(cliaddr);	// size of the client address
		
		recv_len = recvfrom(sockfd, // socket file descriptor
			recv_buffer,       // receive buffer
			sizeof(recv_buffer),  // max number of bytes to be received
			0,
			(struct sockaddr *) &cliaddr,  // client address
			&len);             // length of client address structure
		
		// start of receive handling
		if (recv_buffer[0] != MAGIC_1 || recv_buffer[1] != MAGIC_2) {
			// Bad message!!! Skip this iteration.
			puts("error! bad message server!");
			continue;

		} else {
			if (recv_buffer[2] == OPCODE_POST) {
				m=0;	// since only an ack is being sent, setting m to 0
				opcode = OPCODE_PACK;	// setting opcode to post ack val
				time_t now = time(NULL);	// for my timestamp	
				char *timeStr = ctime(&now);
				
				memset(recent_msg, 0, sizeof(recent_msg));
				memcpy(recent_msg, recv_buffer + 4, recv_buffer[3]);
				
				// prints to console
				printf("[%s] %s",
					inet_ntoa(cliaddr.sin_addr),
					recv_buffer+4);
				
				// write to file
				fp = fopen("/tmp/test.txt", "w+");	// opens the text file
				fprintf(fp, "<%.24s> [%s:%d] post#%s\n", 
					ctime(&now),
					inet_ntoa(cliaddr.sin_addr),
					cliaddr.sin_port,
					recv_buffer+4);
				fflush(fp);
				fclose(fp);

			} else if (recv_buffer[2] == OPCODE_RETR) {
				opcode = OPCODE_RACK;	// sets the opcode to retrieve ack
				
				// reads in the information from the text file
				fp = fopen("/tmp/test.txt", "r");
				fgets(retrv_msg, 255, (FILE*)fp);
				
				// retstr is set to only the text AFTER the #
				retstr = strchr(retrv_msg, '#') + 1;
				m = strlen(retstr);
				memcpy(send_buffer+4, retstr, m);

				fclose(fp);
				
			} else {
				puts("wrong message format, skip");
				// Wrong message format. Skip this iteration.
				continue;
			}
		}
		
		if (recv_len <= 0) {
			printf("recvfrom() error: %s.\n", strerror(errno));
			return -1;
		}
		// You are supposed to call the sendto() function here to send back
		// the echoed message, using "cliaddr" as the destination address.
		if(recv_len>0){
			send_buffer[0] = MAGIC_1; // These are constants I define
			send_buffer[1] = MAGIC_2;
			send_buffer[2] = opcode;
			send_buffer[3] = m;
			
			sendto(sockfd,		// the socket file descriptor
				send_buffer,	// the sending buffer
				sizeof(send_buffer),		// the number of bytes you want to send
				0,
				(struct sockaddr *) &cliaddr, // client address
				sizeof(cliaddr));             // length of client address		
		}
    }
    fflush(fp);
	close(sockfd);
	return 0;
}
