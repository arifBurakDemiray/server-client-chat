#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "secrets.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "time_lib.h"
#include "color.h"
#include "commands.h"
#include <signal.h>

char Time[20] = "";

// SIGINT signal handler to close program when CTRL+C pressed
static volatile sig_atomic_t keep_running = 1;
int incoming = 0;
static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

/**
 * @brief This function is thread function to run receive on a thread for incomings
 * 			why name comm? communicate
 * @param socketId 
 * @return void* 
 */
void * comm(void * socketId){

	int socket = *((int *) socketId);

	while(keep_running){

		char data[BUFFER_SIZE];
		int read = recv(socket,data,BUFFER_SIZE,0); //reads incoming and responses size
		data[read] = '\0';
		if(strcmp(data,DISCONNECT) == 0){
			incoming = 1;
			pthread_exit(NULL);
		}
		else{
			StrTime(Time);
        	fprintf(stderr, "[" CYN "NEW MSG" RESET "][%s]%s\n", Time, data);
		}
		

	}
	pthread_exit(NULL);
}

int main(){

	int client = socket(PF_INET, SOCK_STREAM, 0);

	struct sigaction psa;
    psa.sa_handler = sig_handler;
    sigaction(SIGINT, &psa, NULL);

	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(client, (struct sockaddr*) &server, sizeof(server)) == -1) {
		StrTime(Time);
        fprintf(stderr, "[" RED "ERROR" RESET "][%s] Could not connect to server\n", Time);
        exit(1);
	}

	StrTime(Time);
	fprintf(stderr, "[" GRN "INFO" RESET "][%s] Connected to the server\n", Time);
	fprintf(stderr, "[" GRN "INFO" RESET "] To see the available users type \"contracts\" \n");
	fprintf(stderr, "[" GRN "INFO" RESET "] To chat with a user type \"chat <USER_NAME> <MESSAGE>\" \n");

	pthread_t thread; //this thread does receiving ops
	pthread_create(&thread, NULL, comm, (void *) &client );

	while(keep_running && !incoming){

		char input[BUFFER_SIZE];
		scanf("%s",input); //take command input

		if(strcmp(input,CLIENTS) == 0){
				//list clients
			send(client,input,BUFFER_SIZE,0);

		}
		if(strcmp(input,CHAT) == 0){

			send(client,input,BUFFER_SIZE,0);
			//take name of the client and message
			scanf("%s",input);
			send(client,input,BUFFER_SIZE,0);
			
			scanf("%[^\n]s",input); //eliminate new line
			send(client,input,BUFFER_SIZE,0);

		}

	}

	StrTime(Time);
	fprintf(stderr, "[" GRN "INFO" RESET "][%s] Disconnecting...\n", Time);
	if(incoming != 1){
		send(client,DISCONNECT,6,0);
	}
	close(client);
	return 0;
}