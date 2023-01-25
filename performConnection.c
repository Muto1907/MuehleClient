#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <sys/epoll.h>

#include "performConnection.h"
#include "shmConnectorThinker.h"

#define wordlength 128
#define POSITIONLENGTH 8
#define BUF 1024
#define CLIENTVERSION "VERSION 3.1\n"
#define EXIT_ERROR  (-1)
#define MAXPLAYERNUMBER 48
#define READ_SIZE 7
#define MAX_EVENTS 4

	
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
    gameInfo = malloc(sizeof(GAMEINFO));

    //filling gameInfo data
    set_GameParam(gameInfo);

    //Creating and attaching shared memory segment for actual communication with Thinker
    //needs current number of players given in gameInfo
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
    shm_allPlayerInfo[0] = (PLAYERINFO *) (shm_gameInfo+1); //pointing to address after shm_gameInfo
    *shm_allPlayerInfo[0] = *allPlayerInfo[0]; 
    for(int i=1; i < shm_gameInfo->countPlayer; i++){
        shm_allPlayerInfo[i]  = (PLAYERINFO *) shm_allPlayerInfo[i-1]+1; //pointing to address that is sizeof(PLAYERINFO) greater than shm_allPlayerInfo[0] 
        *shm_allPlayerInfo[i] = *allPlayerInfo[i]; 
    } 

    //PLAYERINFO *shm_allPlayerInfo[shm_gameInfo->countPlayer]; //pointer to player info; actual number of players taken into account
    
    //TEST send signal to thinker
    //kill(shm_gameInfo->idThinker, SIGUSR1);
    //printf("Connector: SIGUSR1 sent\n");

    //cleanup

    //free memory for GAMEINFO *gameInfo and PLAYERINFO *allPlayerInfo
    for(int i=0; i< gameInfo->countPlayer; i++){
        free(allPlayerInfo[i]);
    } 
    free(gameInfo);
} 


//entire TCP Protocoll
int performConnection(int fileDescriptor, char* gameID, PARAM_CONFIG_T* cfg, int *initial_shm, int tc_pipe[]){

    //char moveCommand [BUF];

	// create Client message for gameID
	char formatgameID [18];
	strcpy(formatgameID, "ID ");
	strcat(formatgameID, gameID);
	strcat(formatgameID, "\n");

    int serverMessageCount = 0;

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
                                allPlayerInfo[myPlayerNumber] = malloc(sizeof(PLAYERINFO));
                                set_MyPlayerParam(allPlayerInfo[myPlayerNumber]);
                            }

                            //sixth ++ Server-Message: Total Player Number and Data of enemy Players
                            else if(sscanf(line, "+ TOTAL %d", &playerCount) == 1){
                                printf("SERVER: Number of participating Players: %d\n", playerCount);
                                for(int j = 0; j < playerCount; j++){
                                    if (j == myPlayerNumber) continue;
                                    strcpy(line, linesOfServerMsg[i +j]);
                                    if(sscanf(line, "+ %d %s %d", &enemyPlayerNumber, enemyPlayerName, &isReady) == 3){
                                        //Filling the Struct with enemyPlayer info
                                        allPlayerInfo[j] = malloc(sizeof(PLAYERINFO));
                                        set_EnemyPlayerParam(allPlayerInfo[j]);
                                        if(isReady){
                                            printf("SERVER: Player Number %d (%s) is ready!\n", enemyPlayerNumber, enemyPlayerName);
                                        }
                                        else{
                                            printf("SERVER: Player Number %d (%s) isn't ready yet!\n", enemyPlayerNumber, enemyPlayerName);
                                        }
                                    }
                                }
                                /*after prologue: all relevant data is availabe --> setup can be finished
                                by creating actual shared memory segment and filling the structs*/

                                finishSetup(initial_shm);
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
                                allPlayerInfo[myPlayerNumber]->piecesToBeCaptured = piecesToBeCaptured;
                            }

                            else  if(sscanf(line,"+ PIECELIST %d,%d", &playerCount, &piecesCount) == 2){
                                printf("Server: Number of pieces for each Player: %d\n", piecesCount);
                            }

                            else if(sscanf(line,"+ PIECE%d.%d %s", &playerNumber, &pieceNumber, piecePosition) == 3){
                                //pieceNumber = atoi(pieceNumberstr);
                                strcpy(shm_allPlayerInfo[playerNumber]->piece[pieceNumber].pos, piecePosition);
                                shm_allPlayerInfo[playerNumber]->piece[pieceNumber].piecenum = pieceNumber;
                                shm_allPlayerInfo[playerNumber]->piece[pieceNumber].playerNum = playerNumber;
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
                                }   

                            else if(strcmp(line, "+ WAIT") == 0){
                                printf("SERVER: %s\n", line);
                                sendMsgToServer(fileDescriptor, "OKWAIT\n");
                            }


                            else if(strcmp(line, "+ OKTHINK") == 0 ){
                                //sending signal to thinker
                                shm_gameInfo->flagProvideMove = true;
                                kill(gameInfo->idThinker, SIGUSR1);
                                printf("SERVER: %s\n", serverMsg);
                                //TEST


                            }

                            else if(strcmp(line, "+ MOVEOK") == 0){
                                printf("SERVER: %s\n", line);
                            }


                            else if(strcmp(line, "+ GAMEOVER")==0){
                                printf("GAMEOVER!!!\n");
                            }

                            else if(sscanf(line, "+ PLAYER0WON %s", winner) == 1){
                                if(strcmp(winner, "Yes")){
                                    //TO DO Struct erweitern um Winnerdaten zu speichern
                                    allPlayerInfo[0]->isWinner = 1;
                                    memset(winner, 0, POSITIONLENGTH);
                                }
                            }
                            
                            else if(sscanf(line, "+ PLAYER1W0N %s", winner) == 1){
                                if(strcmp(winner, "Yes")){
                                    //TO DO Struct erweitern um Winnerdaten zu speichern
                                    allPlayerInfo[1]->isWinner = 1;
                                    memset(winner, 0, POSITIONLENGTH);
                                }
                            }

                            else if(strcmp(line, "+ QUIT") == 0){
                                if(allPlayerInfo[0]->isWinner && !allPlayerInfo[1]->isWinner){
                                    printf("%s IS THE WINNER!!!! CONGRATULATIONS\n", allPlayerInfo[0]->playerName);
                                }
                                else if(!allPlayerInfo[0]->isWinner && allPlayerInfo[1]->isWinner){
                                    printf("%s IS THE WINNER!!!! CONGRATULATIONS\n", allPlayerInfo[1]->playerName);
                                }
                                else{
                                    printf("IT'S A DRAW!!!! WELL PLAYED %s and %s\n", allPlayerInfo[0]->playerName, allPlayerInfo[1]->playerName);
                                }

                                if(close(epoll_fd)){
                                perror("Failed to close epoll file descriptor.\n");
                                return -1;
                                }

                                else printf("closing epoll\n");
                                
                                //detach shm here
                                return 0;

                            }
                        
                        }

                    }     
                }
            }
            else if(events[j].data.fd == tc_pipe[0]){
                    if(read(tc_pipe[0], &move, READ_SIZE + 1) == -1){
                        perror("connector process can't read from pipe.\n");
                        return -1;
                    }
                    else{
                        //printf("This is the move: %s\n", move);
                        //memset(moveCommand, 0, BUF);
                        
                        //strcpy(moveCommand, move);
                        //strcat(moveCommand, "\n");

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
