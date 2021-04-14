/**
 * udp_client.c
 * @author Jennifer Norby
 * @version 10/28/2019
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * definition of constants
 */
#define MAGIC_1 0x4A // ascii for J
#define MAGIC_2 0x4E // ascii for N
#define OPCODE_POST 0x01 // post
#define OPCODE_PACK 0x02 // post ack
#define OPCODE_RETR 0x03 // retrieve
#define OPCODE_RACK 0x04 // retrieve ack

/*
 * start of main
 */
int main(int argc, char *argv[]) {
	int ret;
	int sockfd = 0;
	char send_buffer[1024];
	char recv_buffer[1024];
	char user_input[1024];
	char recent_msg[201];
	int recv_len;
	socklen_t len;

	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("socket() error: %s.\n", strerror(errno));
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(32000);

	ret = connect(sockfd,
		(struct sockaddr *) &serv_addr,
		sizeof(serv_addr));
		
	if (ret < 0) {
		printf("connect() error: %s.\n", strerror(errno));
		return -1;
	}

	while (1) {
		int m = 0; // initilize m to 0
		len = sizeof(serv_addr);	// length of server address
		
		fgets(user_input,
			sizeof(user_input),
			stdin);
		
		// start of send handling
		if (strncmp(user_input, "post#", 5) == 0) {
			m = strlen(user_input) - 5;
			memcpy(send_buffer+4, user_input + 5, m);

			send_buffer[0] = MAGIC_1; // These are constants I define
			send_buffer[1] = MAGIC_2;
			send_buffer[2] = OPCODE_POST;
			send_buffer[3] = m;
			
			sendto(sockfd,		// the socket file descriptor
				send_buffer,	// the sending buffer
				m+5,		// the number of bytes you want to send
				0,
				(struct sockaddr *) &serv_addr, // destination address
				sizeof(serv_addr));             // size of the address
		
		} else if (strncmp(user_input, "retrieve#", 9) == 0) {
			// Check whether it matches to "retrieve#"
			// Note that a retrieve message has no payload, e.g., m=0
			m = 0;
			memcpy(send_buffer+4, user_input + 9, m);

			send_buffer[0] = MAGIC_1; // These are constants I define
			send_buffer[1] = MAGIC_2;
			send_buffer[2] = OPCODE_RETR;
			send_buffer[3] = m;
			
			sendto(sockfd,		// the socket file descriptor
				send_buffer,	// the sending buffer
				m+9,		// the number of bytes you want to send
				0,
				(struct sockaddr *) &serv_addr, // destination address
				sizeof(serv_addr));             // size of the address
		} else {
			//	if it does not match any known command, skip iteration
			// 	and print out an error message.
			puts("Error: Unrecognized command format");
			continue;
		}
		
		recv_len = recvfrom(sockfd, // socket file descriptor
			recv_buffer,       // receive buffer
			sizeof(recv_buffer),  // max number of bytes to be received
			0,
			(struct sockaddr *) &serv_addr,  // client address
			&len);             // length of client address structure
			
		if (recv_len <= 0) {
			printf("recvfrom() error: %s.\n", strerror(errno));
			return -1;
		}
		
		// Checks the user input format
		if (recv_buffer[0] != MAGIC_1 || recv_buffer[1] != MAGIC_2) {
				// Bad message!!! Skip this iteration.
				puts("error bad message client");
				continue;

		} else {
			if (recv_buffer[2] == OPCODE_PACK) {
				// posts this if post is acknowledged by server
				puts("post_ack#successful");

			} else if (recv_buffer[2] == OPCODE_RACK) {
				// posts this if retrieve was acknowledge by server and returns the 
				//		data from the text file
				printf("retrieve_ack#%s", recv_buffer+4);
				
			} else {
				puts("wrong message format, skip");
				// Wrong message format. Skip this iteration.
				continue;
			}
		}
	}

	close(sockfd);
	return 0;
}
