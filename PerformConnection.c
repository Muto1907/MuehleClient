#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>


#define BUF 1024
#define gameVersion "Version 2.1\n"
	
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
		printf("Error: %d n", result);
		exit(EXIT_FAILURE);
		}
		if(bytesReceived >= BUF){
			printf("Buffer overflow");
			break;
		}
		// Make sure the serve Message is complete if not receive again
	} while (serverMsg[bytesReceived - 1] != '\n');
	printf("Client: %s\n", serverMsg);
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







int performConnection(int fileDescriptor, char* gameID){

	// create Client message for gameID
	char formatgameID [18];
	strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

	//Receive firsst Server-Message
	getServermsg(fileDescriptor);

	//TODO: Formatierte Ausgabe der ersten Servernachricht GameServer Version
	

	if(serverMsg[0] == '-'){
		printf("Error: %s\n", serverMsg);
	}
	else if(serverMsg[0] =='+'){

		sendMsgToServer(fileDescriptor, gameVersion);	
	}

		//Empfange zweite Nachricht vom Server
	getServermsg(fileDescriptor);
		//TODO: Formatierte Ausgabe der zweiten Servernachricht GAME ID

	if(getServermsg[0] == '-'){
		printf("Error: %s\n", serverMsg);

	}
	else if(serverMsg[0] == '+'){
		sendMsgToServer(fileDescriptor, formatgameID);
	}

		//Empfange dritte Nachricht vom Server
	getServermsg(fileDescriptor);
		//TODO: Formatierte Ausgabe der dritten Servernachricht Gamekind-Name

	if (serverMsg[0] == '-'){
		printf("Error: %s\n", serverMsg);
	}
	if (strcmp(serverMsg,"+ PLAYING NMMorris\n") != 0){
			printf("Error: Wrong Game selected!\n");
	}

		//Empfange vierte Nachricht vom Server
	getServermsg(fileDescriptor);
		//TODO: Formatierte Ausgabe der vierten Servernachricht Game-Name
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}

		//Sende Mitspielernummer an Server 
		sendMsgToServer(fileDescriptor, "PLAYER\n");


		//Empfange fünfte Nachricht vom Server Spielerinfo
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}

		//Empfange sechste Nachricht vom Server Mitspieleranzahl
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}

		//Empfange siebte Nachricht vom Server Mitspieler BEREIT
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}

		//Empfange achte Nachricht vom Server ENDPLAYERS
	getServermsg(fileDescriptor);
		if (serverMsg[0] == '-'){
			printf("Error: %s\n", serverMsg);
		}
	return 0;
}





