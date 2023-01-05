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
#define POSITIONLENGTH 4
#define BUF 1024
#define CLIENTVERSION "VERSION 3.1\n"
#define EXIT_ERROR  (-1)
#define MAXPLAYERNUMBER 48

	
char serverMsg[BUF];
char clientMsg[BUF];
char** linesOfServerMsg;
char *tokenArray[wordlength];
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
int maxTurnTime;
int piecesToBeCaptured;
int piecesCount;
int pieceNumber;
char piecePosition[POSITIONLENGTH];
int playerNumber;
char pieceNumberstr[POSITIONLENGTH];
char playerNumberstr[POSITIONLENGTH];

//initialize structs

GAMEINFO *gameinfo;
PLAYERINFO *playerinfo[MAXPLAYERNUMBER];




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
    //char pseudoscan[BUF];

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
                    printf("ERROR Number %d: %s\n",serverMessageCount, serverMsg);
                    exit(EXIT_ERROR);
                    break;
            }
        }
        //+ means the Server is giving a positive Response
        else if(serverMsg[0] =='+'){
            linesOfServerMsg = serverMsgToLines(serverMsg,tokenArray);
            for(int i = 0; i < wordlength; i++){
                char *line = linesOfServerMsg[i];
                if(linesOfServerMsg[i] != NULL){
                    
                    //first Servermessage
                    if(sscanf(line, "+ MNM Gameserver %s accepting connections", gameServerVersion) == 1){
                        printf("SERVER: Welcome to MNM Gameserver the Gameserver Version is: %s\n",gameServerVersion);
                        sendMsgToServer(fileDescriptor, CLIENTVERSION);	
                    }
        
                
                    //second Server-Message: Client Version
                    else if(strcmp(line, "+ Client version accepted - please send Game-ID to join") == 0){
                        printf("SERVER: %s\n", serverMsg);
                        sendMsgToServer(fileDescriptor, formatgameID);
                    }

                    //third and fourth Server-Message: Gamekindname and GameName
                    else if(sscanf(line, "+ PLAYING %s",gameKindName) == 1){
                        if(strcmp(gameKindName, cfg->gamename) != 0){
                            printf("Error: Wrong Game selected!\n");
                        }
                        strcpy(gameName, linesOfServerMsg[i+1]);
                        
                        printf("SERVER: Playing %s\nSERVER: GameName: %s\n", gameKindName,gameName); 
                        
                        //Sending empty Playernumber means Server decides which number we get. 
                        sendMsgToServer(fileDescriptor, "PLAYER\n");      
                    }
                    
                    //fifth Server-Message: our Playernumber and Playername
                    else if(sscanf(line, "+ YOU %d %[^\t\n]", &myPlayerNumber, myPlayerName)){
                        printf("SERVER: Your Playernumber: %d\nSERVER: Your Playername: %s\n", myPlayerNumber, myPlayerName);
                        playerinfo[myPlayerNumber] = setMyPlayerParam();
                    }

                    //sixth ++ Server-Message: Total Player Number and Data of enemy Players
                    else if(sscanf(line, "+ TOTAL %d", &playerCount) == 1){
                        printf("SERVER: Number of participating Players: %d\n", playerCount);
                        for(int j = 0; j < playerCount; j++){
                            if (j == myPlayerNumber) continue;
                            strcpy(line, linesOfServerMsg[i +j]);
                            if(sscanf(line, "+ %d %s %d", &enemyPlayerNumber, enemyPlayerName, &isReady) == 3){
                                //Filling the Struct with enemyPlayer info
                                playerinfo[j] = setEnemyPlayerParam();
                                if(isReady){
                                    printf("SERVER: Player Number %d (%s) is ready!\n", enemyPlayerNumber, enemyPlayerName);
                                }
                                else{
                                    printf("SERVER: Player Number %d (%s) isn't ready yet!\n", enemyPlayerNumber, enemyPlayerName);
                                }
                            }
                        }
                    }
                
                //End of Prologue

                    else if(strcmp(line, "+ ENDPLAYERS") == 0){
                        printf("SERVER: ENDPLAYERS means the Prologue is over.\n");
                    }
                    
                    else if(sscanf(line,"+ MOVE %d", &maxTurnTime) == 1){
                        printf("Server: Time to think: %d ms.\n", maxTurnTime);
                    }
                        

                    else  if(sscanf(line,"+ CAPTURE %d", &piecesToBeCaptured) == 1){
                        printf("Server: Pieces to be captured: %d\n", piecesToBeCaptured);
                    }

                    else  if(sscanf(line,"+ PIECELIST %d,%d", &playerCount, &piecesCount) == 2){
                        printf("Server: Number of pieces for each Player: %d\n", piecesCount);
                    }

                    else if(sscanf(line,"+ PIECE%d.%d %s", &playerNumber, &pieceNumber, piecePosition) == 3){
                        //pieceNumber = atoi(pieceNumberstr);
                        strcpy(playerinfo[playerNumber]->piece[pieceNumber].pos, piecePosition);
                        playerinfo[playerNumber]->piece[pieceNumber].piecenum = pieceNumber;
                        printf("PIECE%d.%d %s\n",playerNumber, pieceNumber, piecePosition);
                        memset(piecePosition,0,POSITIONLENGTH);
                        //memset(pieceNumberstr,0, POSITIONLENGTH);
                    }
                        

                        /*else if(sscanf(line,"+ PIECE1.%s %s", pieceNumberstr, piecePosition) == 2){
                            pieceNumber = atoi(pieceNumberstr);
                            //strcpy(playerinfo[0]->piece[pieceNumber].pos, piecePosition);
                            printf("PIECE1.%d %s\n", pieceNumber, piecePosition);
                            memset(piecePosition,0,POSITIONLENGTH);
                            memset(pieceNumberstr,0, POSITIONLENGTH);
                        }*/

                    else if(strcmp(line, "+ ENDPIECELIST") == 0){
                        printf("SERVER: ENDPIECESLIST.\n");
                        sendMsgToServer(fileDescriptor,"THINKING\n");
                        //printf("Message received\n");
                        /*free(playerinfo[0]);
                        free(playerinfo[1]);*/
                        }   

                    else if(strcmp(line, "+ WAIT") == 0){
                        sendMsgToServer(fileDescriptor, "OKWAIT\n");
                    }


                    else if(strcmp(line, "+ OKTHINK") == 0 ){
                        printf("SERVER: %s\n", serverMsg);
                    }

                    //else if(strcmp())
                
                }

            }     
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

PLAYERINFO* setMyPlayerParam()
{
    PLAYERINFO* playerInfo = malloc(sizeof(PLAYERINFO));
    strcpy (playerInfo->playerName, myPlayerName);
    playerInfo->playerNumber = myPlayerNumber;
    playerInfo->ready = 1;
    return playerInfo;

}
PLAYERINFO* setEnemyPlayerParam()
{

    PLAYERINFO* enemyPlayerInfo = malloc(sizeof(PLAYERINFO));
    strcpy(enemyPlayerInfo->playerName, enemyPlayerName);
    enemyPlayerInfo->playerNumber = enemyPlayerNumber;
    enemyPlayerInfo->ready = isReady;
    return enemyPlayerInfo;
}

char** serverMsgToLines(char* server_Msg, char** linesOfServerMsg){
    memset(linesOfServerMsg, 0, BUF);
    char *saveptr;
    int numToken = 0;
    char *token;
    char *presentToken = server_Msg;

    while((token = strtok_r(presentToken,"\n", &saveptr)) != NULL){
        linesOfServerMsg[numToken] = token;
        numToken++;
        presentToken = NULL;
    }

    return linesOfServerMsg;

     
}