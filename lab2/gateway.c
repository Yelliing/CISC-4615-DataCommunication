
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
#define MAXLINE 65535 

// Driver code 
int main() {
	int sockfd;
	int r;
	int crc = 0;	// integer to store the crc number
	char buffer[MAXLINE-5], sendline[MAXLINE];	
	struct sockaddr_in gateaddr, cliaddr,sinkaddr;
	
	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	/* Step 1: Declare a variable of type cm_t. Declare another variable          */
	cm_t cm;
	p_cm_t p_cm = &cm;

	/* Step 2: Assign values to the parameter fields of the structure.            */
	// this is the stardard parameter for 16bit
	p_cm->cm_width = 16;                                            
	p_cm->cm_poly  = 0x8005L;                                       
	p_cm->cm_init  = 0L;                                            
	p_cm->cm_refin = TRUE;                                          
	p_cm->cm_refot = TRUE;                                          
	p_cm->cm_xorot = 0L;        

	/* Step 3: Initialize the instance with a call cm_ini(p_cm);                  */
	cm_ini(p_cm);

	memset(&gateaddr, 0, sizeof(gateaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	memset(&sinkaddr, 0, sizeof(sinkaddr));

	// Filling server information 
	gateaddr.sin_family = AF_INET; // IPv4 
	gateaddr.sin_addr.s_addr = INADDR_ANY;
	gateaddr.sin_port = htons(PORT);

	sinkaddr.sin_family = AF_INET; // IPv4 
	sinkaddr.sin_addr.s_addr = INADDR_ANY;
	sinkaddr.sin_port = htons(PORT + 1000);

	// Bind the socket with the server address 
	if (bind(sockfd, (const struct sockaddr *)&gateaddr,
		sizeof(gateaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listening...\n\n");
	int len1 = sizeof(gateaddr), len2 = sizeof(sinkaddr), n;

	while (1) {
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len1);
		/* Step 4: Process zero or more message bytes by placing zero or more successive calls to cm_nxt.        */
		for (int i=0; i <= strlen(buffer); i++)
			cm_nxt(p_cm,buffer[i]);
		/* Step 5: Extract the CRC value */
		crc = cm_crc(p_cm);

		r = rand() % 4;	// generate random integer among 5 so that there is a 0.2 possiblility
		if (r == 1)
			buffer[0] = '0';
		buffer[n] = '\0';
		printf("Client : %s\n", buffer);
		sprintf(sendline, "%s%d", buffer, crc);
		sendto(sockfd, (const char *)sendline, strlen(sendline), MSG_CONFIRM, (const struct sockaddr *) &sinkaddr, len2);
		printf("Message sent.\n");
	}

	return 0;
}