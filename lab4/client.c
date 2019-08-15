#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>


#define MYPORT  6660
#define BUFFER_SIZE 1024
#define Max_Message_Size 8
#define TH_FIN  1    // we need this one
#define TH_SYN  2   // we need this one
#define TH_RST  4
#define TH_PUSH 5
#define TH_ACK  6   // we need this one
#define TH_URG  7



typedef struct {
  int th_seq;
  int th_ack;
  int th_flags;
  int th_win;
}tcphdr;

typedef struct {
  tcphdr my_tcphdr;
  //vector<char> payload(Max_Message_Size);
  char payload[Max_Message_Size];
}message;


int main()
{
	char *establish[BUFFER_SIZE];
	char *closing[BUFFER_SIZE];
	char sendline[BUFFER_SIZE];
    ///sockfd
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

	//send the syn message
    message *my_message=(message*)malloc(sizeof(message));
    my_message->my_tcphdr.th_seq=0;
    my_message->my_tcphdr.th_ack=10000;
    my_message->my_tcphdr.th_win=8;
    my_message->my_tcphdr.th_flags=TH_SYN;
    strcpy(my_message->payload, "Hello");

	//send sync message from server
	send(sock_cli, establish, BUFFER_SIZE, 0);
	my_message->my_tcphdr.th_seq += 1;
	printf("Send SYN.\n");

	//send back acknowlegement to server
	if (recv(sock_cli,establish, BUFFER_SIZE, 0))
	{
		printf("Recieive SYN-ACK.\n");
		my_message->my_tcphdr.th_ack += 1;
		
		send(sock_cli, establish, BUFFER_SIZE, 0);
		printf("Send ACK, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, my_message->my_tcphdr.th_win);
		my_message->my_tcphdr.th_flags = TH_ACK;
		my_message->my_tcphdr.th_seq += 1;
		printf("Connection established.\n");
	}

    char fname[100];
    FILE *fptr; // the pointer to open file
	// printf("Please enter the message to send:\n");
	// fgets(sendline, 1024, stdin);
        printf("Enter the file name to send to server: \n");
        scanf("%s", &fname);  // let the user enter the file name to send to server

        // read a file from program.txt
        if ((fptr = fopen(fname, "r")) == NULL) {
            printf("Error! File does not exist.");

            // Program exits if the file pointer returns NULL.
            exit(1);
        }
        fgets(sendline, BUFFER_SIZE, fptr);   // read the file to sendline


    int needSend = strlen(sendline);
	
    char *buffer=(char*)malloc(needSend);
   // memcpy(buffer,sendline,len);

    int pos=0;
    int len=1;
	int packet=1;
    send(sock_cli, sendline, needSend,0);

    while(pos < needSend)
    {
    	my_message->my_tcphdr.th_seq += len;
        my_message->my_tcphdr.th_ack += len;
    	//memcpy(buffer,sendline,len);
        //send(sock_cli, buffer+pos, len,pos);
        printf("Sending packet%d, Seq = %d , len = %d.\n", packet, my_message->my_tcphdr.th_seq, len);
        if(len <= 0)
        {
            perror("ERROR");
            break;
        }
        pos+=len;
        if (len < Max_Message_Size)
        	len = len*2;
        packet +=1;
        //slow_start(&len, Max_Message_Size);  //determine the packet size
    }

   // printf("I sent a Type %d message\n", my_message->my_tcphdr.th_flags);
    // free(buffer);
    // free(my_message);

	


	
	//strcpy(*closing, "Bye");

	//send finish message from server
	
	send(sock_cli, closing, BUFFER_SIZE, 0);
	my_message->my_tcphdr.th_seq += 1;
    my_message->my_tcphdr.th_flags = TH_FIN;
	printf("Send FIN, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, my_message->my_tcphdr.th_win);
	
	if (recv(sock_cli, closing, BUFFER_SIZE, 0))
	{
		printf("Recieive FIN-ACK.\n");
		my_message->my_tcphdr.th_ack += 1;
		if (recv(sock_cli, closing, BUFFER_SIZE, 0)) {
			my_message->my_tcphdr.th_ack += 1;
			printf("Recieive FIN.\n");		
			my_message->my_tcphdr.th_flags = TH_ACK;
			my_message->my_tcphdr.th_seq += 1;
			send(sock_cli, closing, BUFFER_SIZE, 0);
			printf("Send last ACK, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, my_message->my_tcphdr.th_win);		
		}
	}
	


	free(buffer);
	free(my_message);
    close(sock_cli);
    printf("Send over!!!\n");
    return 0;
}
