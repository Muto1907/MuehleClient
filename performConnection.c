#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <sys/epoll.h>
#include <stdbool.h>

#include "performConnection.h"
#include "shmConnectorThinker.h"

#define wordlength 128
#define POSITIONLENGTH 16
#define BUF 1024
#define CLIENTVERSION "VERSION 3.1\n"
#define EXIT_ERROR  (-1)
#define MAXPLAYERNUMBER 48
#define READ_SIZE 24
#define MAX_EVENTS 4

	
bool firstTime = true;
char serverMsg[BUF];
char clientMsg[BUF];
char** linesOfServerMsg;
char *tokenArray[wordlength];
//variables for receiving Servermessage:
size_t size;
int msgSnippet;
int bytesReceived;

//variables for epoll and pipes
char buffer[READ_SIZE +1];
int event_count = 0;
struct epoll_event event_sock, event_pipe, events[MAX_EVENTS];



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
char winner[POSITIONLENGTH];
char move[POSITIONLENGTH];
char rest[wordlength];

int shm_id;
//initialize structs
//initial structs before shared memory is created
GAMEINFO *gameInfo;
PLAYERINFO *allPlayerInfo[MAXPLAYERNUMBER];

//structs in shared memory
GAMEINFO *shm_gameInfo;
PLAYERINFO *shm_allPlayerInfo[MAXPLAYERNUMBER]; 




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
		printf("CLIENT: %s", clientMsg);
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
        gameInfo->enemyPlayerNumber = enemyPlayerNumber;
    }
}

void set_MyPlayerParam(PLAYERINFO *playerInfo)
{ 
    strcpy(playerInfo->playerName, myPlayerName);
    playerInfo->playerNumber = myPlayerNumber;
    playerInfo->ready = 1;
    playerInfo->isWinner = false;
}

void set_EnemyPlayerParam(PLAYERINFO *playerInfo)
{
    strcpy(playerInfo->playerName, enemyPlayerName);
    playerInfo->playerNumber = enemyPlayerNumber;
    playerInfo->ready = isReady;
    playerInfo->isWinner = false;
}

void finishSetup(int *initial_shm_ptr){

    //filling gameInfo data
    set_GameParam(gameInfo);

    //Creating and attaching shared memory segment for actual communication with Thinker
    //needs current number of players given in gameInfo

    shm_id = createShm(gameInfo);
    
    //initial_shm_ptr points to shm_id
    *initial_shm_ptr = shm_id;
    

    void *shmPtr_connector;
    shmPtr_connector = attachShm(shm_id);

    

    //creating pointer to addresses in actual shm segment where game info and player infos are stored respectively

    //pointer to game info
    shm_gameInfo = (GAMEINFO *) shmPtr_connector;
    *shm_gameInfo = *gameInfo;


    //pointer to player info array
    shm_allPlayerInfo[0] = (PLAYERINFO *) (shm_gameInfo+1); //pointing to address after shm_gameInfo

    *shm_allPlayerInfo[0] = *allPlayerInfo[0]; 
    for(int i=1; i < gameInfo->countPlayer; i++){
        shm_allPlayerInfo[i]  = (PLAYERINFO *) shm_allPlayerInfo[i-1]+1; //pointing to address that is sizeof(PLAYERINFO) greater than shm_allPlayerInfo[0] 
        *shm_allPlayerInfo[i] = *allPlayerInfo[i]; 
    } 
    PIECEINFO * firstFreeAddress = (PIECEINFO *) (shm_allPlayerInfo[gameInfo->countPlayer-1]+1);
    //pointer to piece lists
    for(int i = 0;i < gameInfo->countPlayer; i++){
        shm_allPlayerInfo[i]->piece = (PIECEINFO *) (firstFreeAddress+i*(gameInfo->piecesCount));
    }



} 


//entire TCP Protocoll
int performConnection(int fileDescriptor,int getoptPlayerNum, char* gameID, PARAM_CONFIG_T* cfg, int *initial_shm, int tc_pipe[]){
    bool gameOver = false;
    //char moveCommand [BUF];

	// create Client message for gameID
	char formatgameID [24];
    memset(formatgameID, 0, 24);
    strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

    // Message to Request the Playernumber from Server
    char playerNumCommand [wordlength];    
    memset(playerNumCommand, 0, wordlength);

 
    int epoll_fd = epoll_create1(0);



        if(epoll_fd == -1){
            perror("Failed to create epoll file descriptor.\n");
            return -1;
        }

        //adds  pipe file descriptors to epoll to check for changes
        event_pipe.events = EPOLLIN;
        event_pipe.data.fd = tc_pipe[0];
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tc_pipe[0], &event_pipe)){
            perror("Failed to add file descriptor to epoll.\n");
            close(epoll_fd);
            return -1;
        }
    

        //adds socket file descriptor to epoll to check for changes
        event_sock.events = EPOLLIN;
        event_sock.data.fd = fileDescriptor;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileDescriptor, &event_sock)){
            perror("Failed to add file descriptor to epoll.\n");
            close(epoll_fd);
            return -1;
        }
        

    while (1){
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int j = 0; j < event_count; j++){
            if(events[j].data.fd == fileDescriptor){       
                    
                getServermsg(fileDescriptor);


                
                //if serverMessage begins with '-' an Error occured.
                if(serverMsg[0] == '-'){
                    perror(serverMsg);
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
                                    perror("Error: Wrong Game selected!\n");
                                }
                                strcpy(gameName, linesOfServerMsg[i+1]);
                              
                                
                                printf("SERVER: Playing %s\nSERVER: GameName: %s\n", gameKindName,gameName); 
                                
                                //Sending empty Playernumber means Server decides which number we get. 
                                if (getoptPlayerNum > 0){
                                    sprintf(playerNumCommand, "PLAYER %d\n", getoptPlayerNum - 1);
                                    sendMsgToServer(fileDescriptor, playerNumCommand);
                                }
                                else{
                                    sendMsgToServer(fileDescriptor, "PLAYER\n"); 
                                }
                                      
                            }
                            
                            //fifth Server-Message: our Playernumber and Playername
                            else if(sscanf(line, "+ YOU %d %[^\t\n]", &myPlayerNumber, myPlayerName)){
                                printf("SERVER: Your Playernumber: %d\nSERVER: Your Playername: %s\n", myPlayerNumber, myPlayerName);
                            }

                            //sixth ++ Server-Message: Total Player Number and Data of enemy Players
                            else if(sscanf(line, "+ TOTAL %d", &playerCount) == 1){
                                printf("SERVER: Number of participating Players: %d\n", playerCount);
                                for(int j = 0; j < playerCount; j++){
                                   

                                        //Filling the Struct with enemyPlayer info
                                        allPlayerInfo[j] = malloc(sizeof(PLAYERINFO));
                                        if(j == myPlayerNumber){
                                            set_MyPlayerParam(allPlayerInfo[myPlayerNumber]);
                                            continue;
                                        }
                                        else{       
                                            if(myPlayerNumber == 0){
                                                sscanf(linesOfServerMsg[i+j], "+ %d %[^\t] %d", &enemyPlayerNumber, enemyPlayerName, &isReady);
                                            }                                         
                                            else{
                                                sscanf(linesOfServerMsg[i+j+1], "+ %d %[^\t] %d", &enemyPlayerNumber, enemyPlayerName, &isReady);
                                            }
                                

                                            enemyPlayerName[strlen(enemyPlayerName) -2] = '\0';
                                            set_EnemyPlayerParam(allPlayerInfo[j]);
                                        

                                            if(isReady == 1){
                                                printf("SERVER: Player Number %d (%s) is ready!\n", enemyPlayerNumber, enemyPlayerName);
                                            }
                                            else{
                                                printf("SERVER: Player Number %d (%s) isn't ready yet!\n", enemyPlayerNumber, enemyPlayerName);
                                            }
                                        }
                                    
                                }

                                //filling the structs
                                //local gameInfo only for creation of actual shm segment
                                gameInfo = malloc(sizeof(GAMEINFO));
                            }
                        
                            //End of Prologue

                            else if(strcmp(line, "+ ENDPLAYERS") == 0){
                                printf("SERVER: ENDPLAYERS means the Prologue is over.\n");
                            }
                            
                            else if(sscanf(line,"+ MOVE %d", &maxTurnTime) == 1){
                                printf("SERVER: Time to think: %d ms.\n", maxTurnTime);
                            }
                                

                            else  if(sscanf(line,"+ CAPTURE %d", &piecesToBeCaptured) == 1){
                                printf("SERVER: Pieces to be captured: %d\n", piecesToBeCaptured);
                                gameInfo->piecesToBeCaptured = piecesToBeCaptured;
                            }

                            else  if(sscanf(line,"+ PIECELIST %d,%d", &playerCount, &piecesCount) == 2){
                                printf("SERVER: Number of pieces for each Player: %d\n", piecesCount);
                                gameInfo->piecesCount = piecesCount;


                                /*after prologue: all relevant data is availabe --> setup can be finished
                                by creating actual shared memory segment and filling the structs*/
                                if (firstTime){
                                finishSetup(initial_shm);
                                firstTime=false;
                                }
                                shm_gameInfo->piecesToBeCaptured = piecesToBeCaptured;
                            }

                            else if(sscanf(line,"+ PIECE%d.%d %s", &playerNumber, &pieceNumber, piecePosition) == 3){
                                strcpy(shm_allPlayerInfo[playerNumber]->piece[pieceNumber].pos, piecePosition);
                                shm_allPlayerInfo[playerNumber]->piece[pieceNumber].piecenum = pieceNumber;
                                shm_allPlayerInfo[playerNumber]->piece[pieceNumber].playerNum = playerNumber;
                                printf("PIECE%d.%d %s\n",playerNumber, pieceNumber, piecePosition);
                                memset(piecePosition,0,POSITIONLENGTH);
                            }

                            else if(strcmp(line, "+ ENDPIECELIST") == 0){
                                printf("SERVER: ENDPIECESLIST.\n");
                                if(!gameOver)
                                sendMsgToServer(fileDescriptor,"THINKING\n");
                                
                                }   

                            else if(strcmp(line, "+ WAIT") == 0){
                                printf("SERVER: %s\n", line);
                                sendMsgToServer(fileDescriptor, "OKWAIT\n");
                            }


                            else if(strcmp(line, "+ OKTHINK") == 0 ){
                                //sending signal to thinker
                                shm_gameInfo->flagProvideMove = true;

                                kill(shm_gameInfo->idThinker, SIGUSR1);
                                printf("SERVER: %s\n", serverMsg);


                            }

                            else if(strcmp(line, "+ MOVEOK") == 0){
                                printf("SERVER: %s\n", line);
                            }


                            else if(strcmp(line, "+ GAMEOVER")==0){
                                gameOver = true;
                                printf("GAMEOVER!!!\n");

                            }

                            else if(sscanf(line, "+ PLAYER%dWON %s",&playerNumber, winner) == 2){

                                if(strcmp(winner, "Yes") == 0){
                                    shm_allPlayerInfo[playerNumber]->isWinner = true;

                                }
                                memset(winner, 0, POSITIONLENGTH);
                            }

                            else if(strcmp(line, "+ QUIT") == 0){

                                for(int i = 0; i <shm_gameInfo->countPlayer; i++){
                                    if (shm_allPlayerInfo[i]->isWinner){
                                        if((i + 1) == shm_gameInfo->countPlayer){
                                            if(shm_allPlayerInfo[i]->isWinner){
                                                printf("%s IS THE WINNER!!!! CONGRATULATIONS\n", shm_allPlayerInfo[i]->playerName);
                                            }
                                        }
                                        else{
                                            if (shm_allPlayerInfo[i+1]->isWinner){
                                            printf("IT'S A DRAW!!!! WELL PLAYED EVERYONE\n");
                                            break;
                                            }
                                            else{
                                                printf("%s IS THE WINNER!!!! CONGRATULATIONS\n", shm_allPlayerInfo[i]->playerName);
                                            }
                                        }
                                        
                                    }
                                }
                                
                               

                                if(close(epoll_fd)){
                                perror("Failed to close epoll file descriptor.\n");
                                return -1;
                                }
                                
                                //detach shm here
                                //free memory for GAMEINFO *gameInfo and PLAYERINFO *allPlayerInfo
                                for(int i=0; i< gameInfo->countPlayer; i++){
                                    free(allPlayerInfo[i]);
                                } 
                                free(gameInfo);
                                //clearShm(shm_id);
                                return 0;

                            }
                            else{
                                printf("Not expected: %s\n", line);
                            }
                        
                        }

                    }     
                }
            }
            else if(events[j].data.fd == tc_pipe[0]){
                    if(read(tc_pipe[0], &move, POSITIONLENGTH) == -1){
                        perror("connector process can't read from pipe.\n");
                        return -1;
                    }
                    else{

                        sendMsgToServer(fileDescriptor, move);
                    }                
            }
        }
    }
    return 0;
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
