#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include "performConnection.h"

#define BUF 1024
#define GAMEVERSION "VERSION 2.1\n"

	
char serverMsg[BUF];
char clientMsg[BUF];
//variables for receiving Servermessage:
size_t size;
int msgSnippet;
int bytesReceived;


// Receive the server message
void getServermsg(int fileDescriptor){
	//Reset previous Server message
	size = 0;
	msgSnippet = 0;
	bytesReceived = 0;
	memset(serverMsg, 0, BUF);

	do {
		size = recv(fileDescriptor, &serverMsg[bytesReceived], BUF - bytesReceived, 0);
		msgSnippet++;
		if(size > 0) {
			bytesReceived += size;
		}
		else{
		printf("Error: %ld n", size);
		exit(EXIT_FAILURE);
		}
		if(bytesReceived >= BUF){
			printf("Buffer overflow");
			break;
		}
		// If the server message isn't complete receive again
	} while (serverMsg[bytesReceived - 1] != '\n');
	
}


void sendMsgToServer(int fileDescriptor, char* msgInput) {
	strcpy(clientMsg, msgInput);
	if(send(fileDescriptor, clientMsg, strlen(clientMsg), 0) < 0){
		printf("Error Client message could not be sent.\n");
	} else {
		printf("Client: %s", clientMsg);
	}
	// clean up clientMsg for later use
	memset(clientMsg, 0, BUF);
}

//Prologue of the TCP Protocoll
int performConnection(int fileDescriptor, char* gameID){

	// create Client message for gameID
	char formatgameID [18];
	strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

	//Receive first Server-Message: Gameserverversion
	getServermsg(fileDescriptor);

	//TODO Errorhandling and Output (server message prints are just a placeholder for now)
	

	if(serverMsg[0] == '-'){
		printf("Error: %s\n", serverMsg);
	}
	else if(serverMsg[0] =='+'){
		printf("Server: %s\n", serverMsg);
		sendMsgToServer(fileDescriptor, GAMEVERSION);	
	}

		//receive second Server-Message: Client Version
	getServermsg(fileDescriptor);
		

	if(serverMsg[0] == '-'){
		printf("Error: %s\n", serverMsg);

	}
	else if(serverMsg[0] == '+'){
		printf("Server: %s\n", serverMsg);
		sendMsgToServer(fileDescriptor, formatgameID);
	}

		//Receive third and fourth Server-Message: Gamekindname, Gamename 
	getServermsg(fileDescriptor);


	if (serverMsg[0] == '-'){
		printf("Error: %s\n", serverMsg);
	}

	//TODO Error Wrong gamekindname
	if (strncmp(serverMsg,"+ PLAYING NMMorris\n", 19) != 0){
			printf("Error: Wrong Game selected!\n");
	}

	
	else if (serverMsg[0] == '+'){
		printf("Server: %s\n", serverMsg);
	//Sending empty Playernumber means Client decides which number we get. 
		sendMsgToServer(fileDescriptor, "PLAYER\n");
	}
	

	//receive fifth Server-Message: Playernumber, Playername
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}
		else if(serverMsg[0] == '+'){
			printf("Server: %s\n", serverMsg);
		}


	//receive sixth Server-Message: Total number of Players 
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}
		else if(serverMsg[0] == '+'){
			printf("Server: %s\n", serverMsg);
		}
		//receive seventh Server-message: Players ready
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}
		else if(serverMsg[0] == '+'){
			printf("Server: %s\n", serverMsg);
		}

		//receive eight Server-Message ENDPLAYERS
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}
		else if(serverMsg[0] == '+'){
			printf("Server: %s\n", serverMsg);
		}
	return 0;
}





