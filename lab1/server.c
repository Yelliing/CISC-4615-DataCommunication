/* Lab 1
   Author : Xiao Cai
   Date: 9-25-18

   description: all functions working properly. Some minor problems does appear. 
   1. After a few commands the list function return value has excess string following the original value, 
   2. the history command returns the history along with two unknown character
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 4088    // the port users will be connecting to
#define BACKLOG 5     // how many pending connections queue will hold
#define BUF_SIZE 200
int fd_A[BACKLOG];    // accepted connection fd
int conn_amount;    // current connection amount

void showclient()
{
	int i;
	printf("client amount: %d\n", conn_amount);
	for (i = 0; i < BACKLOG; i++) {
		printf("[%d]:%d  ", i, fd_A[i]);
	}
	printf("\n\n");
}

int main(void)
{
	int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_in server_addr;    // server address information
	struct sockaddr_in client_addr; // connector's address information
	socklen_t sin_size;
	int yes = 1;

	char buf[BUF_SIZE];
	char buf1[BUF_SIZE];
	char buf2[BUF_SIZE];
	char buf_list[BUF_SIZE];
	char buf_msg[BUF_SIZE];
	char buf_content[BUF_SIZE];
	char buf_his[BUF_SIZE];
	char buf_history[BUF_SIZE][BUF_SIZE][BUF_SIZE];

	int ret;
	int i, j, x, cnt;
	char *HIS;
	char *ID;
	int ID_num, HIS_num;
	char command1[] = "list";
	char command2[] = "history";
	char command3[] = "exit";
	char command4[] = "msg";

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	server_addr.sin_family = AF_INET;         // host byte order
	server_addr.sin_port = htons(MYPORT);     // short, network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock_fd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("listen port %d\n", MYPORT);

	fd_set fdsr;
	int maxsock;
	struct timeval tv;
	x = cnt = 0;
	conn_amount = 0;
	sin_size = sizeof(client_addr);
	maxsock = sock_fd;
	while (1) {
		// initialize file descriptor set
		FD_ZERO(&fdsr);
		FD_SET(sock_fd, &fdsr);

		// timeout setting
		tv.tv_sec = 30;
		tv.tv_usec = 0;

		// add active connection to fd set
		for (i = 0; i < BACKLOG; i++) {
			if (fd_A[i] != 0) {
				FD_SET(fd_A[i], &fdsr);
			}
		}

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			break;
		}
		else if (ret == 0) {
			printf("timeout\n");
			continue;
		}

		// check every fd in the set
		for (i = 0; i < conn_amount; i++) {
			if (FD_ISSET(fd_A[i], &fdsr)) {
				ret = recv(fd_A[i], buf, sizeof(buf), 0);
				if (ret <= 0) {        // client close
					printf("client[%d] close\n", i);
					--conn_amount;
					close(fd_A[i]);
					FD_CLR(fd_A[i], &fdsr);
					fd_A[i] = 0;
				}
				else {        // receive data
					if (ret < BUF_SIZE)
						memset(&buf[ret], '\0', 1);
					printf("client[%d] send:%s\n", fd_A[i], buf);
					char* tempstr = calloc(strlen(buf) + 1, sizeof(char));
					strcpy(tempstr, buf);

					if (strncmp(ID = strtok(buf, " "), command4, strlen(command4)) == 0)  // function for msg
					{
						memset(buf_content, 0, strlen(buf_content));
						memset(buf1, 0, strlen(buf1));
						ID = strtok(NULL, " ");
						ID_num = atoi(ID);
						
						
						while (ID != NULL) {  // seperate all the string by space
							ID = strtok(NULL, " ");
							if (ID != NULL) {
								strcat(buf_content, " ");
								strcat(buf_content, ID);	//
							}
						}
						
						for (int k = 0; k < conn_amount; ++k){
							if (ID_num == fd_A[k]) {
								strcpy(buf1, "Message sent.\n");
								strcpy(buf_history[fd_A[k]][x++], buf_content);  // copy this message into the history array
								++cnt;			// count the message sent
								send(fd_A[k], "You received a message: ", strlen("You received a message: "), 0);  // did not use strcpy to prevent this prompt show up in the history
								send(fd_A[k], buf_content, strlen(buf_content), 0);	// send the message to the client with the correct id
								break;
							}
							else {
								memset(buf_his, 0, strlen(buf_his));
								strcpy(buf1, "Invalid ID.\n");	// when the id is not valid do not proceed
							}
								
						}
						send(fd_A[i], buf1, strlen(buf1), 0);  // give respond to the clien that gave the command
						puts(buf1);
					}


					// function for list
					if (strncmp(strtok(buf, " "), command1, strlen(command1)) == 0) 
					{
					memset(buf_list, 0, strlen(buf_list));
					strcpy(buf_list, "The active IDs are --- ");
					for (j = 0; j < conn_amount; j++)
						if (fd_A[j] != 0)
						{
							char c[4];
							sprintf(c, "%d\t", fd_A[j]);
							strcat(buf_list, c);
						}
					strcat(buf_list, "\nEND");
					send(fd_A[i], buf_list, strlen(buf_list), 0);
					}
					

					//function for history
					if (strncmp(HIS = strtok(tempstr, " "), command2, strlen(command2)) == 0)
					{
						memset(buf_his, 0, strlen(buf_his));
						HIS = strtok(NULL, " ");    // find only once for the next char after space for user id
						HIS_num = atoi(HIS);				
						for (int k = 0; k < conn_amount; ++k) {
							if (HIS_num == fd_A[k]) {
								strcpy(buf2, "\n History sent.\n");
								strcat(buf_his, "History is here: ");
								for (x = 0; x < cnt; x++) {     // loop through the string stored in the history string
									strcat(buf_his, "\n");
									strcat(buf_his, buf_history[fd_A[k]][x]);   //add them at the back of a string 
								}
								send(fd_A[i], buf_his, strlen(buf_his), 0); // send the string to client 
								break;
							}
							else {
								memset(buf_his, 0, strlen(buf_his));
								strcpy(buf2, "\nInvalid ID.\n");	// when the id is not valid do not proceed
							
							}
								
						}
						send(fd_A[i], buf2, strlen(buf2), 0);   // confirm whether the client entered a correct id
						puts(buf2);
					}



					// function for exit
					if (strncmp(strtok(buf, " "), command3, strlen(command3)) == 0)
					{
						send(fd_A[i], "Goodbye", 8, 0);
						--conn_amount;     // reduce the client amount
						close(fd_A[i]);
						FD_CLR(fd_A[i], &fdsr);
						fd_A[i] = 0;
					}
				}
			}
		}

		// check whether a new connection comes
		if (FD_ISSET(sock_fd, &fdsr)) {
			new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0) {
				perror("accept");
				continue;
			}

			// add to fd queue
			if (conn_amount < BACKLOG) {
				fd_A[conn_amount++] = new_fd;
				printf("new connection client[%d] %s:%d\n", conn_amount,
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				if (new_fd > maxsock)
					maxsock = new_fd;
			}
			else {
				printf("max connections arrive, exit\n");
				send(new_fd, "bye", 4, 0);
				close(new_fd);
				break;
			}
		}
		showclient();
	}

	// close other connections
	for (i = 0; i < BACKLOG; i++) {
		if (fd_A[i] != 0) {
			close(fd_A[i]);
		}
	}

	exit(0);
}
