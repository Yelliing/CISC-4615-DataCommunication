
// Server side implementation of UDP 
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
#define MAXLINE 65535 // 65，535 byte is the largest size that udp can handle

// Driver code 
int main() {
	int sockfd, num=0, crc=0, start;
	char buffer[MAXLINE];
	struct sockaddr_in gateaddr,sinkaddr;

	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	/* Step 1: Declare a variable of type cm_t. Declare another variable          */
	cm_t cm;
	p_cm_t p_cm = &cm;
	/* Step 2: Assign values to the parameter fields of the structure.            */
	p_cm->cm_width = 16;
	p_cm->cm_poly = 0x8005L;
	p_cm->cm_init = 0L;
	p_cm->cm_refin = TRUE;
	p_cm->cm_refot = TRUE;
	p_cm->cm_xorot = 0L;

	cm_ini(p_cm);

	memset(&gateaddr, 0, sizeof(gateaddr));
	memset(&sinkaddr, 0, sizeof(sinkaddr));

	// Filling server information 
	sinkaddr.sin_family = AF_INET; // IPv4 
	sinkaddr.sin_addr.s_addr = INADDR_ANY;
	sinkaddr.sin_port = htons(PORT + 1000);

	// Bind the socket with the server address 
	if (bind(sockfd, (const struct sockaddr *)&sinkaddr,
		sizeof(sinkaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listening...\n\n");
	int len, n;
	while (1) {
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &gateaddr, &len);
		buffer[n] = '\0';
		start = strlen(buffer) - 5;	// the beginning position of crc

		char *crc_c = (char*)malloc(6);	//extract the crc
		strncpy(crc_c, buffer + start, 5);

		for (int i = start; i <= strlen(buffer); ++i) {
				buffer[i] = 0;
		}//remove crc from string
		
		//generate crc again to compare with the extracted one
		for (int i = 0; i <= strlen(buffer); i++)
			cm_nxt(p_cm, buffer[i]);
		crc = cm_crc(p_cm);

		sscanf(crc_c, "%d", &num);
		printf("the crc received is %d \n the crc generated is %d \n", crc, num);
		// compare both crc to check the string if they are equal
		if (crc == num) {
			printf("Messege received correctly. \n");
			printf("Gateway : %s\n\n", buffer);
		}
		else
			printf("Messege corrupted. \n\n");
		
	}

	return 0;
}
