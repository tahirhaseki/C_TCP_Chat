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
    char name[20];
    int isPrivate;
    char* password;
    Client* owner;
    struct ChatRoom* next;
    struct ChatRoom* prev;
} typedef ChatRoom;

ChatRoom* initializeChatRoom(char *name,Client* owner,int isPrivate,char* password){
    ChatRoom* r = malloc(sizeof(ChatRoom));
    r->id = ++roomdId;
    strcpy(r->name,name);
    r->isPrivate = isPrivate;
    r->owner = owner;
    r->next = NULL;
    r->prev = NULL;
    r->password = calloc(1,20);
    if(password != NULL)
        strcpy(r->password,password);
    return r;
}

Client *clientRoot,*clientCurrent;
ChatRoom *roomRoot,*roomCurrent;

char* memConcat(const char* s1,const char s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    return result;
}

void printRoomsToScreen(int clientInfo){
    ChatRoom *tempRoom = roomRoot;
    Client *tempClient = clientRoot;
    char *temp = malloc(200);
    char *room = malloc(200);
    while(tempRoom != NULL){
        bzero(room,sizeof(room));
        if(!(tempRoom->isPrivate)){
            sprintf(temp,"%d | %s | Users: ",tempRoom->id,tempRoom->name);
            strcat(room,temp);
            tempClient = clientRoot;
            while(tempClient != NULL){
                if(tempClient->currentRoom == tempRoom->id){
                    sprintf(temp,"%s ",tempClient->username); 
                    strcat(room,temp);
                }
                tempClient = tempClient->next;
            }
            strcat(room,"\n");
        }
        else{
            sprintf(temp,"%d | %s | Private\n",tempRoom->id,tempRoom->name);
            strcat(room,temp);
        }
        tempRoom = tempRoom->next;
        send(clientInfo,room,200,0);
    }
}
void sendMessageToRoom(int client,int roomID,char * message){
    Client *temp = clientRoot->next;
    while(temp!= NULL){
        if(temp->currentRoom == roomID && temp->connectionInfo != client)
            write(temp->connectionInfo,message,200);
        temp=temp->next;
    }
}

void *connection_handler(void *newClient)
{
    char *username;
    char recv_buffer[200];
    char send_buffer[200];

    Client *client = (Client *)newClient;   

    strcpy(send_buffer,"Hello Client, Welcome to DEU Chat\nPlease enter a username(Min 3 Max 10 character): ");
    write(client->connectionInfo , send_buffer , sizeof(send_buffer));
    while(1){
        read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
        username = strtok(recv_buffer, "\n"); 
        if(strlen(username) < 3 || strlen(username) > 10){
            strcpy(send_buffer,"Invalid username, please enter a valid one: \n");
            write(client->connectionInfo , send_buffer , sizeof(send_buffer));
        }
        else{
            strcpy(client->username,username);
            sprintf(send_buffer,"Welcome to DEU CHAT %s\n",client->username);
            write(client->connectionInfo , send_buffer , sizeof(send_buffer));
            break;
        }
    }   
    while (1)
    {
        bzero(recv_buffer,sizeof(recv_buffer));
        read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
        char * tempRecv = malloc(200);
        strcpy(tempRecv,recv_buffer);
        char* content;
        char *command = strtok(tempRecv, " "); 
        content = strtok(NULL, "\n"); 
        if(client->currentRoom == -1){
            if(strcmp(command,"-list\n") == 0 || strcmp(command,"-list") == 0){
                printRoomsToScreen(client->connectionInfo);
            }
            else if(strcmp(command,"-create") == 0){
                int notUnique = 0;
                ChatRoom* newRoom = initializeChatRoom(content,client,0,NULL);
                ChatRoom *temp = roomRoot;
                while(temp != NULL){
                    if(strcmp(temp->name,content)==0){
                        write(client->connectionInfo,"Room Name must be unique.\n",27);
                        notUnique = 1;
                        break;
                    }
                    temp = temp->next;
                }
                if(notUnique){
                    continue;
                }
                newRoom->prev = roomCurrent;
                roomCurrent->next = newRoom;
                roomCurrent = newRoom;
                client->currentRoom = newRoom->id;
                write(client->connectionInfo,"clear",6);
                sleep(0.1);
                sprintf(send_buffer,"Room: %s\n",newRoom->name);
                write(client->connectionInfo,send_buffer,200);
                sleep(0.1);
            }
            else if(strcmp(command,"-pcreate") == 0){
                char * roomName = malloc(200);
                strcpy(roomName,content);
                ChatRoom *temp = roomRoot;
                while(temp != NULL){
                    if(strcmp(temp->name,roomName)==0){
                        write(client->connectionInfo,"Room Name must be unique.\n",27);
                        continue;
                    }
                    temp = temp->next;
                }
                write(client->connectionInfo,"Enter password:",16);
                bzero(recv_buffer,sizeof(recv_buffer));
                read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
                content = strtok(recv_buffer, "\n");
                ChatRoom* newRoom = initializeChatRoom(roomName,client,1,content);
                newRoom->prev = roomCurrent;
                roomCurrent->next = newRoom;
                roomCurrent = newRoom;
                client->currentRoom = newRoom->id;
                write(client->connectionInfo,"clear",6);
                sleep(0.1);
                sprintf(send_buffer,"Room: %s\n",newRoom->name);
                write(client->connectionInfo,send_buffer,200);
            }
            else if(strcmp(command,"-enter") == 0){
                ChatRoom *temp = roomRoot;
                while(temp != NULL){
                    if(strcmp(content,temp->name) == 0){
                        if(temp->isPrivate == 0){
                            client->currentRoom = temp->id;
                            write(client->connectionInfo,"clear",6);
                            sleep(0.1);
                            sprintf(send_buffer,"Room: %s\n",temp->name);
                            write(client->connectionInfo,send_buffer,200);
                        }
                        else{
                            write(client->connectionInfo,"Enter password:",16);
                            bzero(recv_buffer,sizeof(recv_buffer));
                            read(client->connectionInfo, recv_buffer, sizeof(recv_buffer));
                            content = strtok(recv_buffer, "\n");
                            if(strcmp(content,temp->password) == 0) {
                                client->currentRoom = temp->id;
                                write(client->connectionInfo,"clear",6);
                                sleep(0.1);
                                sprintf(send_buffer,"Room: %s\n",temp->name);
                                write(client->connectionInfo,send_buffer,200); 
                            }
                            else{
                                write(client->connectionInfo,"Wrong password.\n",17);
                            }
                        }
                        break;
                    }
                    temp = temp->next;
                }
            }
        }
        else{
            if(strcmp(command,"-quit\n") == 0){
                int flag = 0;
                Client *tempClient = clientRoot;
                while(tempClient != NULL){
                    if(tempClient != client && tempClient->currentRoom == client->currentRoom){
                        flag++;
                        break;
                    }
                    tempClient = tempClient->next;
                }
                if(flag == 0){
                    ChatRoom *tempRoom = roomRoot;
                    while(tempRoom->id != client->currentRoom){
                        tempRoom = tempRoom->next;
                    }
                    tempRoom->prev->next = tempRoom->next;
                    if(tempRoom->next != NULL){
                        tempRoom->next->prev = tempRoom->prev;
                    }
                    if(tempRoom == roomCurrent){
                        roomCurrent = tempRoom->prev;
                    }
                    free(tempRoom);
                }
                client->currentRoom = -1;
                write(client->connectionInfo,"clear",6);
                sleep(0.1);
                write(client->connectionInfo,"Lobby\n",6);
            }
            else if(strcmp(command,"-msg") == 0){
                sprintf(send_buffer,"%s : %s\n",client->username,content);
                sendMessageToRoom(client->connectionInfo,client->currentRoom,send_buffer);
            }
            else if(recv_buffer[0] != '-'){
                sprintf(send_buffer,"%s : %s",client->username,recv_buffer);
                sendMessageToRoom(client->connectionInfo,client->currentRoom,send_buffer);
            }
        }
        if(strcmp(command,"-whoami\n") == 0){
            sprintf(send_buffer,"%s\n",client->username);
            write(client->connectionInfo,send_buffer,200);
        }
        else if(strcmp(command,"-exit\n") == 0){
            if(client->next != NULL)
                client->next->prev = client->prev;
            client->prev->next = client->next;
            if(client == clientCurrent){
                clientCurrent = client->prev;
            }
            free(client);
            break;
        }
        /*else if(send(client->connectionInfo,"OK",5,MSG_NOSIGNAL) == -1)
            break;*/

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
    server.sin_port = htons(3205);
     
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }

    clientRoot = initializeClient(socket_desc,server);
    strcpy(clientRoot->username,"system");
    clientCurrent = clientRoot;
    roomRoot = initializeChatRoom("General",clientRoot,0,NULL);
    roomCurrent = roomRoot;
    clientRoot->currentRoom = 1;
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