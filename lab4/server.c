#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HELLO_WORLD_SERVER_PORT    6660
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                1024


#define Max_Message_Size 8
#define TH_FIN  1    // we need this one
#define TH_SYN  2   // we need this one
#define TH_RST  4
#define TH_PUSH 5
#define TH_ACK  6   // we need this one
#define TH_URG  7

void slow_start(int start, int end)
{
    if (start < end)
        start = start * 2;

};

typedef struct {	//this is the tcp header
  int th_seq;
  int th_ack;
  int th_flags;
  int th_win;
}tcphdr;

typedef struct {
  tcphdr my_tcphdr;
  char payload[Max_Message_Size];
}message;


int main(int argc, char **argv)
{
	char *establish[BUFFER_SIZE];
	char *closing[BUFFER_SIZE];
    // set socket's address information
    struct sockaddr_in   server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    // create a stream socket
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    //bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Server Bind Port: %d Failed!\n", HELLO_WORLD_SERVER_PORT);
        exit(1);
    }

    // listen
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Server Listen Failed!\n");
        exit(1);
    }

    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t          length = sizeof(client_addr);

        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        if (new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }

        message *my_message=(message*)malloc(sizeof(message));
		my_message->my_tcphdr.th_seq = 10000;
		my_message->my_tcphdr.th_ack = 0;
		my_message->my_tcphdr.th_win = Max_Message_Size;
		my_message->my_tcphdr.th_flags = TH_SYN;

        int needRecv=sizeof(message);
        char *buffer=(char*)malloc(needRecv);


		//receive sync message from client
		if (recv(new_server_socket, establish, BUFFER_SIZE, 0)) {
			printf("Receive SYN \n");
			my_message->my_tcphdr.th_seq += 1;
			my_message->my_tcphdr.th_flags = TH_ACK;
			send(new_server_socket, establish, BUFFER_SIZE, 0);
			printf("Send ACK, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, my_message->my_tcphdr.th_win);
			my_message->my_tcphdr.th_ack += 1;
		}
		//send back acknowlegement to client
		if (recv(new_server_socket, establish, BUFFER_SIZE, 0)) {
			my_message->my_tcphdr.th_seq += 1;
			printf("Connection established. \n");
		}

        int pos=0;
        int len=1;
	
        char recvline[BUFFER_SIZE];

        recv(new_server_socket, recvline, BUFFER_SIZE,0);
        int needrecv=strlen(recvline);
        int packet =1;

         while(pos < needrecv)
        {
            my_message->my_tcphdr.th_seq += len;
            my_message->my_tcphdr.th_ack += len;
            //memcpy(buffer,sendline,len);
            //send(sock_cli, buffer+pos, len,pos);
            printf("Receive SYN for packet%d, ACK = %d , len = %d.\n",packet, my_message->my_tcphdr.th_ack, len);
            printf("Send ACK for packet%d, Seq = %d , len = %d.\n",packet, my_message->my_tcphdr.th_seq, len);
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
		
   //      while(pos < needRecv)
   //      {
   //          my_message->my_tcphdr.th_ack += 1;
   //          recv(new_server_socket, recvline, 8, 0);
			// my_message->my_tcphdr.th_ack += len;
   //          printf("Send ACK, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, len);
   //          printf("len = %d\n", len);

   //          if (len < 0)
   //          {
   //              printf("Server Recieve Data Failed!\n");
   //              break;
   //          }
   //          pos+=len;
   //          slow_start(len, Max_Message_Size);  //determine the packet size
   //      }

		  printf("The message received is: %s \n", recvline);


		if (recv(new_server_socket, closing, BUFFER_SIZE, 0)) {
			my_message->my_tcphdr.th_seq += 1;
			my_message->my_tcphdr.th_flags = TH_ACK;
			printf("Receive FIN \n");

			my_message->my_tcphdr.th_ack += 1;

			send(new_server_socket, closing, BUFFER_SIZE, 0);
			printf("Send ACK, Seq = %d , window = 8.\n", my_message->my_tcphdr.th_seq);
			my_message->my_tcphdr.th_seq += 1;
			my_message->my_tcphdr.th_flags = TH_FIN;
			printf("Send last ACK, Seq = %d , window = %d.\n", my_message->my_tcphdr.th_seq, my_message->my_tcphdr.th_win);
      send(new_server_socket, closing, BUFFER_SIZE, 0);
		}
		
        close(new_server_socket);
        memcpy(my_message,buffer,needRecv);

      //  printf("This is a Type %d message\n", my_message->my_tcphdr.th_flags);

        free(buffer);
        free(my_message);
    }



    close(server_socket);
    return 0;
}
