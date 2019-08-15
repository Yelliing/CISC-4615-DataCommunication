// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "crcmodel.h"

#define PORT     8080 
#define MAXLINE 65535  // 65，535 byte is the largest size that udp can handle

// Driver code 
int main() {
	int sockfd;
	char sendline[MAXLINE];
	struct sockaddr_in    gateaddr;

	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {	
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&gateaddr, 0, sizeof(gateaddr)); // clearing the socket

	// Filling gateway information 
	gateaddr.sin_family = AF_INET;
	gateaddr.sin_port = htons(PORT);
	gateaddr.sin_addr.s_addr = INADDR_ANY;

	int n, len, sz;
	char fname[100];
	FILE *fptr; // the pointer to open file
	FILE *fpsz; // to determine the filesize if it fits udp

	//loop that keeps listening on the port
	while (1) {
		printf("Enter the file name to send to server: \n");
		scanf("%s", &fname);  // let the user enter the file name to send to server

		// read a file from program.txt
		if ((fptr = fopen(fname, "r")) == NULL) {
			printf("Error! File does not exist.");

			// Program exits if the file pointer returns NULL.
			exit(1);
		}

		// get the size of the file
		fseek(fptr, 0L, SEEK_END); 
		sz = ftell(fptr);
		rewind(fptr);  //reset the pointer

		if (sz > MAXLINE-5) {
			printf("Error! UDP size exceeded.");
			exit(1);	
		}
		
		fgets(sendline, MAXLINE-5, fptr);	// read the file to sendline

		//send the string to gateway
		sendto(sockfd, (const char *)sendline, strlen(sendline), MSG_CONFIRM, (const struct sockaddr *) &gateaddr, sizeof(gateaddr));

		printf("file sent.\n\n");
		fclose(fptr);
	}

	close(sockfd);
	return 0;
}