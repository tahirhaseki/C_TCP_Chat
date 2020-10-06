#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include<pthread.h>   // for threading, link with lpthread
// Color codes for console.
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Listener function for additional thread.
void listener(void * conn_info){
	char buffer[200];
	while(1){
		bzero(buffer, sizeof(buffer)); 
		read(conn_info, buffer, sizeof(buffer)); 
		if(strcmp(buffer,"clear") == 0){
			system("clear");
			continue;
		}
		printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, buffer);
		fflush(stdout);
	}
}
// Main handler function using for sending data to server.
void handler(int sockfd) 
{ 
	char buff[200]; 
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
		// Below loop reads from console.
		while ((buff[n++] = getchar()) != '\n'); 
		write(sockfd, buff, sizeof(buff));   // Sends input to server.
		if ((strncmp(buff, "-exit", 4)) == 0) { 
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
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(3205); 

	// connect the client socket to server socket 
	if (connect(sockfd, (struct socketaddr*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else{
		printf("connected to the server..\n"); 
		printf("sockfd =  %d\n",sockfd); 
    }
	// function for connection 
	handler(sockfd); 

	// close the socket 
	close(sockfd); 
} 
