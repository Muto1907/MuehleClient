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

#include "errorHandling.h"
#include "performConnection.h"
#include "paramConfig.h"
#include "shmConnectorThinker.h"

/*
#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"
*/

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

    printf("Initial id: %d\n", initial_shm_id);

    int *initial_shm_ptr;
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

        //Creating and attaching shared memory segment for actual communication with Thinker
        int shm_id;
        shm_id = createShm(gameInfo);
        printf("Actual id: %d\n", shm_id);
        //initial_shm_ptr points to shm_id
        *initial_shm_ptr = shm_id;

        void *shmPtr_connector;
        shmPtr_connector = attachShm(shm_id);

        //creating pointer to addresses in actual shm segment where game info and player infos are stored respectively
        GAMEINFO *shm_gameInfo;
        //PLAYERINFO *shm_allPlayerInfo;

        shm_gameInfo = (GAMEINFO *) shmPtr_connector;
        //shm_allPlayerInfo = (PLAYERINFO *)

        *shm_gameInfo = *gameInfo;

        //for testing only
        printf("Connector gameInfo->gameName: %s\n", gameInfo->gameName);
        printf("Connector shm_gameInfo->gameName: %s\n", shm_gameInfo->gameName);

        //free memory for GAMEINFO *gameInfo
        free(gameInfo);

        _exit(0);
    }

    // Thinker Process starts here

    //waiting for child process Connector to create shm segment
    usleep(50000);
    //Attaching actual shared memory segment with id *initial_shm_ptr for internal communication with Connector
    void *shmPtr_thinker;
    shmPtr_thinker = attachShm(*initial_shm_ptr);

    //creating pointer to addresses in acctual shm segment where game info and player infos are stored respectively
    GAMEINFO *shm_gameInfo;
    //PLAYERINFO *shm_allPlayerInfo;

    shm_gameInfo = (GAMEINFO *) shmPtr_thinker;
    //shm_allPlayerInfo = (PLAYERINFO *) 

    //for testing only
    printf("Thinker shm_gameInfo->gameName: %s\n", shm_gameInfo->gameName);
    printf("Thinker shm_gameInfo->countplayer: %d\n", shm_gameInfo->countPlayer);
    
    // avoids orphan and zombie process, wait for child to die
    while(wait(NULL) > 0){
        //empty
    }

    //delete shared memory segment for Connector and Thinker
    clearShm(initial_shm_id);
    clearShm(*initial_shm_ptr);

    return 0;
}

