#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "errorHandling.h"
#include "PerformConnection.h"


#define BUF 1024
#define RET_ERR (-1)

int performConnection(int filedescriptor, char* gameID){

	size_t size;
	char *buffer = (char*) malloc (sizeof(char) * BUF) ;
	char *gameVersionmsg = "VERSION 2.1\n";
	

	//Empfange erste Nachricht vom Server
	size = recv(filedescriptor, buffer, BUF-1, 0);
	
	if(size > 0) buffer[size] = '\0';
	traceMsgRcvd(buffer);	

	if(buffer[0] == '-'){
		printf("Negative Message from Server\n");
		return RET_ERR;
	}

		//Version des Clients abschicken
		strcpy(buffer, gameVersionmsg); 
		send(filedescriptor, buffer, strlen(buffer), 0);	


		//Empfange zweite Nachricht vom Server
		size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer[size] = '\0';
		traceMsgRcvd(buffer);

	if(buffer[0] == '-'){
		printf("Negative Message from Server\n");
		return RET_ERR;
	}
			
		strcpy(buffer, "ID ");
		strcat(buffer, gameID); 
		//Version des Clients abschicken
		send(filedescriptor, buffer, strlen(buffer), 0);	

		//Empfange dritte Nachricht vom Server
		size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;
		}
		if (strcmp(buffer,"+ PLAYING NMMorris\n") != 0){
			printf("Server is not providing MMorris\n");
			return RET_ERR;
		}

		//Empfange vierte Nachricht vom Server
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;
		}

		//Sende Mitspielernummer an Server 
		strcpy(buffer, "PLAYER\n");
		send(filedescriptor, buffer, strlen(buffer), 0);


		//Empfange fünfte Nachricht vom Server Spielerinfo
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;		
		}

		//Empfange sechste Nachricht vom Server Mitspieleranzahl
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;
		}

		//Empfange siebte Nachricht vom Server Mitspieler BEREIT
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;
		}

		//Empfange achte Nachricht vom Server ENDPLAYERS
	size = recv(filedescriptor, buffer, BUF-1, 0);
		if(size > 0) buffer [size] = '\0';
		traceMsgRcvd(buffer);
		if (buffer[0] == '-'){
			printf("Negative Message from Server\n");
			return RET_ERR;
		}
	return 0; // TODO: for testing successful build, unclear what should be returned
}





