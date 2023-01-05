#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "errorHandling.h"
#include "performConnection.h"
#include "paramConfig.h"
#include "shmConnectorThinker.h"
#include "thinking.h"

/*
#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"
*/

void *shmPtr_thinker;
int *initial_shm_ptr;
    
void printAnweisung(){
    printf("-g <GAME-ID> 13-stellige Game-ID\n");
    printf("-p gewünschte Spielernummer\n");
}

/*getSocketDescriptorAndConnect creates a socket and tries to connect to the LMU server.
* returns socket file descriptor socketfd upon success, otherwise -1
*/
int getSocketDescriptorAndConnect(PARAM_CONFIG_T* cfg){
    int socketfd;
    char portnb[4*sizeof(int)];
    snprintf(portnb, sizeof(portnb), "%d", cfg->port); //converting int PORTNUMBER to char-array portnb

    //Retrieving the server address using getaddrinfo()
    struct addrinfo hints, *res, *result;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC; //using IPv4 or IPv6 addresses
    hints.ai_socktype = SOCK_STREAM; //using TCP connection
    hints.ai_protocol = IPPROTO_TCP; //using TCP protocol

    if(getaddrinfo(cfg->hostname, portnb, &hints, &result) != 0) {
        errWithHost ("getaddrinfo", cfg->hostname);
        return -1;
    }

    res = result;
    /*Iterating through the list of address structures obtained by getaddrinfo().
    * Trying to connect to one of the sockets.
    * If socket creation fails, we immediately continue with next iteration (continue).
    * If connection can be established, loop is ended (break).
    * Otherwise, socket gets closed and loop is continued.
    */

    while (res) {

        if((socketfd = socket(res->ai_family, SOCK_STREAM, 0))==-1) {
            continue;
        }

        if(connect(socketfd, res->ai_addr, res->ai_addrlen) != -1) {
            break;
        }

        close(socketfd);

        res = res->ai_next;
    }


    freeaddrinfo(result); //no longer needed since IP-address of server has been resolved

    //All connection attempts failed, i.e. res reached the end of the list of address structures
    if(NULL == res) {
        errWithHost("Connect to server", cfg->hostname);
        return -1;
    }

    return socketfd;
}



void signalHandler(int signal){
    printf("signalHandler %d\n",signal);
    if(signal == SIGUSR1){
        //for test purpose
        printf("thinking... shm_id %d\n",*initial_shm_ptr);
        //Attaching actual shared memory segment with id *initial_shm_ptr for internal communication with Connector
        //void *shmPtr_thinker; -> global variable
        shmPtr_thinker = attachShm(*initial_shm_ptr);
        think(shmPtr_thinker);

         // TODO : free shm(?)
    }
}












int main(int argc,char**argv){

    if (argc <= 1)
    {
        printAnweisung();
        return -1;
    }

    char game_id[15]= {};
    char playernumber[2]={};
    int option;

    PARAM_CONFIG_T  config;
    char    configFile[256];
    InitConfigParam(&config);
    strcpy(configFile,"client.conf");

    while((option = getopt(argc, argv, "g:p:f:")) != -1){
        switch(option){
            case 'g':
            strcpy(game_id, optarg);
            break;
            case 'p':
            strcpy(playernumber, optarg);
            break;
            case 'f':
            strcpy(configFile, optarg);
            break;
            default:
            printAnweisung();
            return 0;
        }
    }

    LoadConfigParam(&config,configFile);
    DumpConfig(&config);

    /*// Test, if game-ID has 13 digits
    int counter = 1;
    long long zahl = game_id;
    while(zahl > 9){
        zahl /= 10;
        counter++;
    }*/

    if(strlen(game_id) != 13){
        errPrintInvalidParam("Game-ID");
        printAnweisung();
        return -1;
    }

    //Test, if playernumber is 1 or 2
    if(strcmp(playernumber, "1") != 0) {
        if(strcmp(playernumber, "2") != 0){
            errPrintInvalidParam("Player number");
            printAnweisung();
            return -1;
        }
    }


    
    //printf("socket fd: %d\n", socketfd); //for testing only!!
    //TODO error handling for socketfd == -1

    pid_t pid; //Process-ID

    int initial_shm_id; //ID for the shared memory pointing to actual shared memory

    //creating and attaching initial shm segment storing the shm id for the actual shm segment
    if((initial_shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1) {

        errFunctionFailed("initial_shm_id creation");
    }

    initial_shm_ptr = (int *) attachShm(initial_shm_id);

    //Connector Process
    if((pid = fork()) == 0){
        //Preparing connection to server "sysprak.priv.lab.nm.ifi.lmu.de"
        int socketfd = getSocketDescriptorAndConnect(&config);        
        //printf("socket fd: %d\n", socketfd); //for testing only!!
        if (socketfd == -1)
            errFunctionFailed ("getSocketDescriptorAndConnect");
        else
            performConnection(socketfd, game_id, &config);
        
        //filling gameInfo data
        GAMEINFO *gameInfo;
        gameInfo = setParam();
        PLAYERINFO* playerInfo[gameInfo->countPlayer];
        playerInfo[0]= setMyPlayerParam();
        playerInfo[1]= setEnemyPlayerParam();

        //Creating and attaching shared memory segment for actual communication with Thinker
        int shm_id;
        shm_id = createShm(gameInfo);
        
        //initial_shm_ptr points to shm_id
        *initial_shm_ptr = shm_id;
        printf("Connector shm_id %d\n",*initial_shm_ptr);

        void *shmPtr_connector;
        shmPtr_connector = attachShm(shm_id);

        //creating pointer to addresses in actual shm segment where game info and player infos are stored respectively
        //pointer to game info
        GAMEINFO *shm_gameInfo;
        shm_gameInfo = (GAMEINFO *) shmPtr_connector;
        *shm_gameInfo = *gameInfo;

        //pointer to player info array
        PLAYERINFO *shm_allPlayerInfo[shm_gameInfo->countPlayer]; //pointer to player info; actual number of players taken into account
        shm_allPlayerInfo[0]  = (PLAYERINFO *) (((GAMEINFO *) shmPtr_connector)+1); //pointing to address that is sizeof(GAMEINFO) greater than shmPtr_connector address
        *shm_allPlayerInfo[0] = *playerInfo[0];
        if(shm_gameInfo->countPlayer == 2) {
            shm_allPlayerInfo[1]  = shm_allPlayerInfo[0]+1; //pointing to address that is sizeof(PLAYERINFO) greater than shm_allPlayerInfo[0] 
            *shm_allPlayerInfo[1] = *playerInfo[1];
        }

        //for testing only
        /*shm_allPlayerInfo[0]->playerNumber = 132;
        if(shm_gameInfo->countPlayer == 2) {
            shm_allPlayerInfo[1]->playerNumber = 465;
        }   */

        
        //for testing only
        printf("Connector gameInfo->gameName: %s\n", gameInfo->gameName);
        printf("Connector shm_gameInfo->gameName: %s\n", shm_gameInfo->gameName);
        printf("Connector shm_allPlayerInfo[0]->playerNumber: %d\n", shm_allPlayerInfo[0]->playerNumber);
         printf("Connector shm_allPlayerInfo[0]->playerName: %s\n", shm_allPlayerInfo[0]->playerName);
        if(shm_gameInfo->countPlayer == 2) {
            printf("Connector shm_allPlayerInfo[1]>playerNumber: %d\n", shm_allPlayerInfo[1]->playerNumber);
             printf("Connector shm_allPlayerInfo[1]->playerName: %s\n", shm_allPlayerInfo[1]->playerName);
        }   
        //TEST send signal to thinker
        kill(gameInfo->idThinker, SIGUSR1);
        printf("Connector: SIGUSR1 sent\n");

        //free memory for GAMEINFO *gameInfo
        free(gameInfo);
        free(playerInfo[0]);
        free(playerInfo[1]);

        _exit(0);
    }

    // Thinker Process starts here
    printf("Thinker after fork, shm_id %d\n",*initial_shm_ptr);
    //Attaching actual shared memory segment with id *initial_shm_ptr for internal communication with Connector
    //void *shmPtr_thinker; -> global variable
    //shmPtr_thinker = attachShm(*initial_shm_ptr);
    //Configure Signalhandling
    signal(SIGUSR1, signalHandler);
    printf("Thinker: registered handler SIGUSR1\n");
    //waiting for child process Connector to create shm segment
    //usleep(1000000);

#if 0
    //creating pointer to addresses in acctual shm segment where game info and player infos are stored respectively
    GAMEINFO *shm_gameInfo;
    shm_gameInfo = (GAMEINFO *) shmPtr_thinker;

    PLAYERINFO *shm_allPlayerInfo[shm_gameInfo->countPlayer];
    shm_allPlayerInfo[0]  = (PLAYERINFO *) (((GAMEINFO *) shmPtr_thinker)+1);
    if(shm_gameInfo->countPlayer == 2) {
        shm_allPlayerInfo[1]  = shm_allPlayerInfo[0]+1; //pointing to address that is sizeof(PLAYERINFO) greater than shm_allPlayerInfo[0] 
    } 


    //for testing only
    printf("Thinker shm_gameInfo->gameName: %s\n", shm_gameInfo->gameName);
    printf("Thinker shm_gameInfo->countplayer: %d\n", shm_gameInfo->countPlayer);
    printf("Thinker shm_allPlayerInfo->playerNumber: %d\n", shm_allPlayerInfo[0]->playerNumber);
    if(shm_gameInfo->countPlayer == 2) {
        printf("Thinker shm_allPlayerInfo[1]>playerNumber: %d\n", shm_allPlayerInfo[1]->playerNumber);
    }   
#endif

    // avoids orphan and zombie process, wait for child to die
    while(wait(NULL) > 0){
        //empty
    }

    //delete shared memory segment for Connector and Thinker
    clearShm(initial_shm_id);
    clearShm(*initial_shm_ptr);

    return 0;
}

