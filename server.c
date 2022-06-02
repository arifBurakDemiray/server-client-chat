// Arif Burak Demiray - 250201022
// contact - arifdemiray@std.iyte.edu.tr

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include "secrets.h"
#include "commands.h"
#include "time_lib.h"
#include "color.h"
#include <signal.h>

#define MAX_SIZE 1024

char Time[20] = "";
int CLIENT_COUNT = 0;

// SIGINT signal handler to close program when CTRL+C pressed

//helper func to get size of char arr
int size(char *values);

//TODO dhke, pool management

struct client{

	int id;
	int socketId;
	struct sockaddr_in address;
	int length;

};

struct client clients[MAX_SIZE];
int free_lots[MAX_SIZE];
pthread_t threads[MAX_SIZE];

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

//slave function like client does receiving
void * comm(void * client){ //receive new connections and commands and respond

	struct client* tempClient = (struct client*) client;
	int socketId = tempClient -> socketId;
	if(socketId == -1)
	{
		StrTime(Time);
		fprintf(stderr, "[" RED "ERROR" RESET "][%s] Cannot accept connections!\n", Time);
		pthread_exit(NULL);
	}

	StrTime(Time); //print client id and port
	fprintf(stderr, "[" GRN "INFO" RESET "][%s] Client connected on socket %d\n", Time, socketId);

	while(keep_running){

		char data[BUFFER_SIZE];
		int read = recv(socketId,data,BUFFER_SIZE,0);
		data[read] = '\0'; //receive command and finish the data by ending it with \0

		char output[BUFFER_SIZE];

		//sigint handler to remove client from pool
		if(strcmp(data,DISCONNECT) == 0){
			pthread_exit(NULL);
		}//send available clients to the desired user
		else if(strcmp(data,CLIENTS) == 0){

			int len = 0;

			for(int i = 0 ; i < CLIENT_COUNT ; i ++){

				if(i != tempClient -> id)
					len += snprintf(output + len,BUFFER_SIZE,"User%d : Socket-%d\n",i + 1,clients[i].socketId);

			}

			send(socketId,output,BUFFER_SIZE,0);
			continue;

		}
		else if(strcmp(data,CHAT) == 0){//because we send data one by one than read it one by one

			read = recv(socketId,data,BUFFER_SIZE,0);
			data[read] = '\0';
										//USERX,userX, UserX
			int receivedId = (data[size(data) - 1] - '0') - 1;

			read = recv(socketId,data,BUFFER_SIZE,0);
			data[read] = '\0';
			char tempArray[BUFFER_SIZE];
			sprintf(tempArray," User%d:%s\0",tempClient->id+1,data); //prepare message

			send(clients[receivedId].socketId,tempArray,BUFFER_SIZE,0);			

		}

	}

	pthread_exit(NULL);

}

int main(){

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sigaction psa; //sigint handler
    psa.sa_handler = sig_handler;
    sigaction(SIGINT, &psa, NULL);

	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = htons(INADDR_ANY);


	if(bind(serverSocket,(struct sockaddr *) &server , sizeof(server)) == -1) {
		StrTime(Time);
        fprintf(stderr, "[" RED "ERROR" RESET "][%s] Could not bind to address\n", Time);
        exit(1);
	}

	if(listen(serverSocket,1024) == -1) {
		StrTime(Time);
        fprintf(stderr, "[" RED "ERROR" RESET "][%s] Cannot listen on socket\n", Time);
        exit(1);
	}

    StrTime(Time);
    fprintf(stderr, "[" GRN "INFO" RESET "][%s] Server started to listen port %d\n", Time,SERVER_PORT);

	while(keep_running){ //continute to accept connections

		clients[CLIENT_COUNT].socketId = accept(serverSocket, (struct sockaddr*) &clients[CLIENT_COUNT].address, &clients[CLIENT_COUNT].length);
		clients[CLIENT_COUNT].id = CLIENT_COUNT;

		pthread_create(&threads[CLIENT_COUNT], NULL, comm, (void *) &clients[CLIENT_COUNT]);

		CLIENT_COUNT ++;
 
	}


	for(int i=0; i< MAX_SIZE; i++){ //send disconnect signal to all clients after finish
		send(clients[i].socketId,DISCONNECT,6,0);
	}


	for(int i = 0 ; i < CLIENT_COUNT ; i ++) //gather all threads
		pthread_join(threads[i],NULL);

	close(serverSocket);
	StrTime(Time);
	fprintf(stderr, "[" GRN "INFO" RESET "][%s] Server is disconnected\n", Time);
	return 0;
}

//calculates size of a char array
//source: https://github.com/arifBurakDemiray/operating-systems/blob/main/ctrl-f/dynamic_array.c
int size(char *values)
{
    int size = 0; //size
    char last;    //current char value
    while (1)
    {
        last = values[size]; //take current char
        if (last != '\0')    //if char not equal empty
            size++;          //increase
        else                 //otherwise means char array finished
            break;
    }
    return size; //return size
}