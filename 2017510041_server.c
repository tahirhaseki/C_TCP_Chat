#include<stdio.h>
#include<string.h>    // for strlen
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#include<unistd.h>    // for write
#include<pthread.h>   // for threading, link with lpthread

struct Client {
    int connectionInfo;
    char ip[16];
    char username[11];
    int currentRoom;
    struct Client *next;
    struct Client *prev;
} typedef Client;

Client* initializeClient(int info,struct sockaddr_in clientInfo){
    Client *c = calloc(1,sizeof(Client));
    c->connectionInfo = info;
    c->currentRoom = -1;
    c->next = NULL;
    c->prev = NULL;
    strcpy(c->ip,inet_ntoa(clientInfo.sin_addr));
    strcpy(c->username,"TBD");
    return c;
}
int roomdId;

struct ChatRoom {
    int id;
    char* name;
    int isPrivate;
    int password;
    Client* owner;
    struct ChatRoom* next;
    struct ChatRoom* prev;
} typedef ChatRoom;

ChatRoom* initializeChatRoom(char *name,Client* owner,int isPrivate){
    ChatRoom* r = malloc(sizeof(ChatRoom));
    r->id = ++roomdId;
    strcpy(r->name,name);
    r->isPrivate = isPrivate;
    r->owner = owner;
    r->next = NULL;
    r->prev = NULL;
    return r;
}

Client *clientRoot,*clientCurrent;
ChatRoom *roomRoot,*roomCurrent;

void printRoomsToScreen(){
    ChatRoom *tempRoom = roomRoot;
    Client *tempClient = clientRoot;
    while(tempRoom != NULL){
        if(!(tempRoom->isPrivate)){
            printf("%d | %s | Users: ",tempRoom->id,tempRoom->name);
            tempClient = clientRoot;
            while(tempClient != NULL){
                if(tempClient->currentRoom == tempRoom->id){
                    printf("%s ",tempClient->username);
                }
                tempClient = tempClient->next;
            }
            printf("\n");
        }
        else{
            printf("%d | %s | Private\n",tempRoom->id,tempRoom->name);
        }
        tempRoom = tempRoom->next;
    }
}

void *connection_handler(void *newClient)
{
    char username[11];
    char recv_buffer[200];
    char send_buffer[200];

    Client *client = (Client *)newClient;   

    strcpy(send_buffer,"Hello Client, Welcome to DEU Chat\nPlease enter a username(Min 3 Max 10 character): ");
    write(client->connectionInfo , send_buffer , sizeof(send_buffer));
    while(1){
        read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
        strcpy(username,recv_buffer);
        if(strlen(username) < 3 || strlen(username) > 10){
            strcpy(send_buffer,"Invalid username, please enter a valid one: ");
            write(client->connectionInfo , send_buffer , sizeof(send_buffer));
        }
        else{
            strcpy(client->username,username);
            sprintf(send_buffer,"Welcome to DEU CHAT %s",client->username);
            write(client->connectionInfo , send_buffer , sizeof(send_buffer));
            break;
        }
    }   
    while (1)
    {
        read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
        if(send(client->connectionInfo,"OK",5,MSG_NOSIGNAL) == -1)
            break;
        printf("Read: %s",recv_buffer);
        if(recv_buffer[0] == '-'){
            printf("Incoming command: %s",recv_buffer);
        }
    }
    
    return 0;
}
void main(){
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    char *message;
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8887);
     
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }

    clientRoot = initializeClient(socket_desc,server);
    strcpy(clientRoot->username,"system");
    clientCurrent = clientRoot;
    listen(socket_desc, 3);
     
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)))
    {
        puts("Connection accepted");

        Client* newClient = initializeClient(new_socket,client);
        newClient->prev = clientCurrent;
        clientCurrent->next = newClient;
        clientCurrent = newClient;

        pthread_t sniffer_thread;
        if(pthread_create(&sniffer_thread, NULL, connection_handler,(void *)newClient))
        {
            puts("Could not create thread");
            return 1;
        }

        puts("Handler assigned");

    }
    puts("Program Ending.");
}