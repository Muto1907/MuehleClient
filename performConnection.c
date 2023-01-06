#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

#include "performConnection.h"
#include "shmConnectorThinker.h"

#define wordlength 128
#define BUF 1024
#define CLIENTVERSION "VERSION 3.1\n"
#define EXIT_ERROR  (-1)
#define MAXPLAYERNUMBER 48
	
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
double maxTimeToMove;
int countPiecesToCapture;
int countPieces;
int currentPlayerNumber;
int currentPieceNumber;
char position[2]; 

//initialising structs in shm
GAMEINFO *shm_gameInfo;
PLAYERINFO **shm_allPlayerInfo;


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

void set_GameParam(GAMEINFO *gameInfo)
{
    if(gameInfo)
    {
        strcpy(gameInfo->gameName, gameKindName);
        gameInfo->myPlayerNumber = myPlayerNumber;
        gameInfo->countPlayer = playerCount;
        gameInfo->idThinker = getppid();
        gameInfo->idConnector = getpid();
    }
}

void set_MyPlayerParam(PLAYERINFO *playerInfo)
{ 
    strcpy(playerInfo->playerName, myPlayerName);
    playerInfo->playerNumber = myPlayerNumber;
    playerInfo->ready = 1;
}

void set_EnemyPlayerParam(PLAYERINFO *playerInfo)
{
    strcpy(playerInfo->playerName, enemyPlayerName);
    playerInfo->playerNumber = enemyPlayerNumber;
    playerInfo->ready = isReady;
}

void finishSetup(int *initial_shm_ptr){
    //filling the structs

    //local gameInfo only for creation of actual shm segment
    GAMEINFO *gameInfo = malloc(sizeof(GAMEINFO));
    /* PLAYERINFO* playerInfo[gameInfo->countPlayer];

    for(int i=0; i<gameInfo->countPlayer; i++){
        playerInfo[i] = malloc(sizeof(PLAYERINFO)); 
    } */

    //filling gameInfo data
    set_GameParam(gameInfo);
    /* setPlayerParam(playerInfo); */

    //Creating and attaching shared memory segment for actual communication with Thinker
    int shm_id;
    shm_id = createShm(gameInfo);
    
    //initial_shm_ptr points to shm_id
    *initial_shm_ptr = shm_id;
    printf("In performConnection: shm_id %d\n",*initial_shm_ptr);

    void *shmPtr_connector;
    shmPtr_connector = attachShm(shm_id);

    //creating pointer to addresses in actual shm segment where game info and player infos are stored respectively

    //pointer to game info
    shm_gameInfo = (GAMEINFO *) shmPtr_connector;
    *shm_gameInfo = *gameInfo;

    //pointer to player info array
    PLAYERINFO *shm_allPlayerInfoTemp[shm_gameInfo->countPlayer]; //pointer to player info; actual number of players taken into account
    *shm_allPlayerInfo = *shm_allPlayerInfoTemp;
    shm_allPlayerInfo[0]  = (PLAYERINFO *) (shm_gameInfo+1); //pointing to address after shm_gameInfo
    set_MyPlayerParam(shm_allPlayerInfo[0]);
    //*shm_allPlayerInfo[0] = *playerInfo[0];
    if(shm_gameInfo->countPlayer == 2) {
        shm_allPlayerInfo[1]  = (PLAYERINFO *) shm_allPlayerInfo[0]+1; //pointing to address that is sizeof(PLAYERINFO) greater than shm_allPlayerInfo[0] 
        set_EnemyPlayerParam(shm_allPlayerInfo[1]);
        //*shm_allPlayerInfo[1] = *playerInfo[1];
    }

    
    //for testing only
    printf("In performConnection: gameInfo->gameName: %s\n", gameInfo->gameName);
    printf("In performConnection: shm_gameInfo->gameName: %s\n", shm_gameInfo->gameName);
    printf("In performConnection: shm_allPlayerInfo[0]->playerNumber: %d\n", shm_allPlayerInfo[0]->playerNumber);
    printf("In performConnection: shm_allPlayerInfo[0]->playerName: %s\n", shm_allPlayerInfo[0]->playerName);
    if(shm_gameInfo->countPlayer == 2) {
        printf("In performConnection: shm_allPlayerInfo[1]->playerNumber: %d\n", shm_allPlayerInfo[1]->playerNumber);
            printf("In performConnection: shm_allPlayerInfo[1]->playerName: %s\n", shm_allPlayerInfo[1]->playerName);
    }   
    //TEST send signal to thinker
    kill(shm_gameInfo->idThinker, SIGUSR1);
    printf("Connector: SIGUSR1 sent\n");

    //cleanup

    //free memory for GAMEINFO *gameInfo
    free(gameInfo);
} 

void writePiecePositionToPiecelist(int playernr, int piecenr, char *position){
    if(playernr == myPlayerNumber){
        shm_allPlayerInfo[0]->piece[piecenr].pos[0] = position[0];
        shm_allPlayerInfo[0]->piece[piecenr].pos[1] = position[1]; 
    } 
    else{
        shm_allPlayerInfo[1]->piece[piecenr].pos[0] = position[0]; 
        shm_allPlayerInfo[1]->piece[piecenr].pos[1] = position[1]; 
    } 
} 

//entire TCP Protocoll
int performConnection(int fileDescriptor, char* gameID, PARAM_CONFIG_T* cfg, int *initial_shm){

	// create Client message for gameID
	char formatgameID [18];
	strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

    int serverMessageCount = 0;
    
    //for sscanf serverMessage without important parameters
    char pseudoscan[BUF];

    //fill the struct with PIDs of thinker and connector Connector = child and Thinker = Parent


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
        //prologue

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

            /*after prologue: all relevant data is availabe --> setup can be finished
            by creating actual shared memory segment and filling the structs*/

            finishSetup(initial_shm);
			
        }

        /*else{
    		printf("Ausserhalb des Prologs: %s", serverMsg);
		}*/

        
        //move command sequence

        else if(sscanf(serverMsg, "+ MOVE %lf\n+ CAPTURE %d\n+ PIECELIST %d,%d\n", &maxTimeToMove, &countPiecesToCapture, &playerCount, &countPieces) == 4){
            printf("Move sequence startet: %lf ms time to make a move.\n", maxTimeToMove);

            for(int i=0; i<countPieces; i++){
                if(sscanf(serverMsg, "+ PIECE%d.%d %s", &currentPlayerNumber, &currentPieceNumber, position)==3){
                    writePiecePositionToPiecelist(currentPlayerNumber, currentPieceNumber, position);
                } 
            } 
        }

        else if(sscanf(serverMsg, "+ ENDPIECELIST\n") == 0){
            printf("Server: ENDPIECELIST means server is waiting for a move\n");
        }  
    }
    }

	return 0;
}