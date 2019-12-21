#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include<pthread.h>   // for threading, link with lpthread
#define MAX 200 
#define PORT 8888 
#define SA struct sockaddr 
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void listener(void * conn_info){
	char buffer[MAX];
	while(1){
		bzero(buffer, sizeof(buffer)); 
		read(conn_info, buffer, sizeof(buffer)); 
		if(strcmp(buffer,"clear") == 0){
			system("clear");
			continue;
		}
		if(strcmp(buffer,"OK") != 0){
			printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, buffer);
		}
		fflush(stdout);
	}
}
void func(int sockfd) 
{ 
	char buff[MAX]; 
	int flag = 1;
	int n; 
	read(sockfd, buff, sizeof(buff)); 
	printf("%s", buff);
	bzero(buff, sizeof(buff));
	pthread_t sniffer_thread;

	for (;;) { 
		bzero(buff, sizeof(buff));
		n = 0; 
		fflush(stdout);
		while ((buff[n++] = getchar()) != '\n') 
            ; 
		write(sockfd, buff, sizeof(buff)); 
		if ((strncmp(buff, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		}
		if(flag){
			if(pthread_create(&sniffer_thread, NULL, listener,(void *)sockfd))
			{
				puts("Could not create thread");
				return;
			}
			flag = 0;
		} 
	} 
} 

int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(8888); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else{
		printf("connected to the server..\n"); 
		printf("sockfd =  %d\n",sockfd); 
    }
	// function for chat 
	func(sockfd); 

	// close the socket 
	close(sockfd); 
} 
