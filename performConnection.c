#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>

#include "performConnection.h"

#define wordlength 128
#define BUF 1024
#define CLIENTVERSION "VERSION 2.1\n"
#define EXIT_ERROR  (-1)

	
char serverMsg[BUF];
char clientMsg[BUF];
//variables for receiving Servermessage:
size_t size;
int msgSnippet;
int bytesReceived;



//parameters to be read from the Server messages
char gameServerVersion[wordlength];
char gameKindName[wordlength];
char gameName[wordlength];
char myPlayerName[wordlength];
int myPlayerNumber;
int playerCount;
char enemyPlayerName [wordlength];
int enemyPlayerNumber;
int isReady;




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
		printf("Error: %ld \n", size);
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
int performConnection(int fileDescriptor, char* gameID, PARAM_CONFIG_T* cfg){

	// create Client message for gameID
	char formatgameID [18];
	strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

    int serverMessageCount = 0;
    
    //for sscanf serverMessage without important parameters
    char pseudoscan[BUF];

    while (1){
	getServermsg(fileDescriptor);
    serverMessageCount++;

	
    //if serverMessage begins with '-' an Error occured.
	if(serverMsg[0] == '-'){

		switch(serverMessageCount){
            case 1 :
                //Error GameServer not accepting connections
                printf("ERROR: Gameserver not responding\n");
                exit(EXIT_ERROR);
                break;
            
            case 2:
                //Error Client Version rejected
                printf("ERROR: Client Version rejected\n");
                exit(EXIT_ERROR);
                break;
            
            case 3:
                //Error wrong GameID
                printf("ERROR: Wrong GameID\n");
                exit(EXIT_ERROR);
                break;
            
            case 4:
                //player Number rejected
                printf("ERROR: Wrong Playernumber\n");
                exit(EXIT_ERROR);
            default: 
                printf("ERROR: %s\n", serverMsg);
                exit(EXIT_ERROR);
                break;
        }
	}
    //+ means the Server is giving a positive Response
	else if(serverMsg[0] =='+'){
        //first Servermessage
		if(sscanf(serverMsg, "+ MNM Gameserver %s accepting connections\n", gameServerVersion) == 1){
	        printf("Server: Welcome to MNM Gameserver the Gameserver Version is: %s\n",gameServerVersion);
            sendMsgToServer(fileDescriptor, CLIENTVERSION);	
        }
    
	
        //second Server-Message: Client Version
		else if(sscanf(serverMsg, "+ Client version accepted - please send %s to join\n", pseudoscan) == 1){
            printf("Server: %s", serverMsg);
		    sendMsgToServer(fileDescriptor, formatgameID);
            memset(pseudoscan, 0, wordlength);
        }
	

		//third and fourth Server-Message: Gamekindname, Gamename 
        else if(sscanf(serverMsg, "+ PLAYING %s\n+ %[^\t\n]",gameKindName, gameName) == 2){
            if(strcmp(gameKindName, cfg->gamename) != 0){
                printf("Error: Wrong Game selected!\n");
            }
            else{
                  printf("Playing: %s\nGameName: %s\n", gameKindName, gameName);
                //Sending empty Playernumber means Server decides which number we get. 
		        sendMsgToServer(fileDescriptor, "PLAYER\n");
              
            }
        }




        //fifth to eighth Server-Message: Message 5 = our Playernumber and Playername, Message 6 = Total player numbers, Message 7 Enemy Player Information, Message 8 ENDPLAYERS 
		else if(sscanf(serverMsg, "+ YOU %d %[^\t\n]\n+ TOTAL %d\n+ %d %s %d\n+ ENDPLAYERS\n",&myPlayerNumber, myPlayerName,&playerCount, &enemyPlayerNumber, enemyPlayerName, &isReady ) == 6){
            printf("Your Playernumber: %d\nYour Playername: %s\nNumber of participating Players: %d\n", myPlayerNumber, myPlayerName, playerCount);
            if(isReady){
                printf("Server: Player Number %d (%s) is ready\n", enemyPlayerNumber, enemyPlayerName);
				printf("Server: ENDPLAYERS means the Prologue is over\n");
            }
            else{
                printf("Server: Player Number %d (%s) isn't ready yet\n", enemyPlayerNumber, enemyPlayerName);
            }
            break;
			
        }

        /*else{
    		printf("Ausserhalb des Prologs: %s", serverMsg);
		}*/
        
    }
    }
	return 0;
}

GAMEINFO* setParam()
{
    GAMEINFO* gameInfo = malloc(sizeof(GAMEINFO));

    if(gameInfo)
    {
        strcpy(gameInfo->gameName, gameKindName);
        gameInfo->myPlayerNumber = myPlayerNumber;
        gameInfo->countPlayer = playerCount;
        gameInfo->idThinker = getppid();
        gameInfo->idConnector = getpid();
    }

    return gameInfo;
}





