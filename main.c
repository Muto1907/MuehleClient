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
#include <sys/epoll.h>

#include "errorHandling.h"
#include "performConnection.h"
#include "paramConfig.h"
#include "shmConnectorThinker.h"
#include "thinking.h"


#define MAX_EVENTS 4
#define READ_SIZE 80

/*
#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"
*/

void *shmPtr_thinker;
int *initial_shm_ptr;
// Thinker -> Connector Pipe
int tc_pipe[2];
    
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
        //let the thinker think and send it to the connector
        //TODO: think based on the last servermsg
        char buff[] = "THINKING\n";
        // thinker writes to pipe
        if(write(tc_pipe[1], buff, strlen(buff) + 1) == -1){
            perror("thinker can't write.\n");
            exit(0);
        }

        //for test purpose
        printf("thinking... shm_id %d\n",*initial_shm_ptr);
        //Attaching actual shared memory segment with id *initial_shm_ptr for internal communication with Connector
        //void *shmPtr_thinker; -> global variable
        shmPtr_thinker = attachShm(*initial_shm_ptr);
        think(shmPtr_thinker);

         // TODO : free shm(?)
    }
}

/// @brief 
/// @param argc 
/// @param argv 
/// @return 
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


    //TODO error handling for socketfd == -1

    pid_t pid; //Process-ID

    int initial_shm_id; //ID for the shared memory pointing to actual shared memory

    //creating and attaching initial shm segment storing the shm id for the actual shm segment
    if((initial_shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1) {

        errFunctionFailed("initial_shm_id creation");
    }

    initial_shm_ptr = (int *) attachShm(initial_shm_id);


    // creating pipe and error handling
    if(pipe(tc_pipe) != 0){
        perror("pipe() failed.\n");
        return -1;
    }

    //Connector Process
    if((pid = fork()) == 0){
        //Preparing connection to server "sysprak.priv.lab.nm.ifi.lmu.de"
        int socketfd = getSocketDescriptorAndConnect(&config);        
       
        if (socketfd == -1)
            errFunctionFailed ("getSocketDescriptorAndConnect");
        else
            performConnection(socketfd, game_id, &config, initial_shm_ptr);


        char buffer[READ_SIZE +1];
        int event_count = 0;
        //size_t bytes_read;
        struct epoll_event event_sock, event_pipe, events[MAX_EVENTS];

        int running = 1, epoll_fd = epoll_create1(0);

        //connector closes write
        close(tc_pipe[1]);

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
        event_sock.data.fd = socketfd;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketfd, &event_sock)){
            perror("Failed to add file descriptor to epoll.\n");
            close(epoll_fd);
            return -1;
        }
        


        printf("\nPolling for input...\n");
        while(running){
            event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            //Debug ready events
            //printf("%d ready events\n", event_count);

            for(int i = 0; i < event_count; ++i){
                //incoming event is from server
                if(events[i].data.fd == socketfd){
                    
                    getServermsg(socketfd);
                    //Debug Socket 
                    printf("Read socket : %s\n", serverMsg);

                    //TODO: Send signal to thinker when the server sends a msg
                    //TEST send signal to thinker
                    kill(getppid(), SIGUSR1);
                    printf("Connector: SIGUSR1 sent\n");

                    //Terminate on negative msg
                    if(strncmp(serverMsg, "-", 1) == 0){
                        running = 0;
                    }

                }
                //incoming event is from pipe
                else if(events[i].data.fd == tc_pipe[0]){
                    if(read(tc_pipe[0], &buffer, READ_SIZE + 1) == -1){
                        perror("connector process can't read from pipe.\n");
                        return -1;
                    }

                    sendMsgToServer(socketfd, buffer);  
                }
                

            }
        }

        if(close(epoll_fd)){
            perror("Failed to close epoll file descriptor.\n");
            return -1;
        }
        _exit(0);

    }

    // Thinker Process starts here
    //close reading pipe for the thinker
    close(tc_pipe[0]);
    printf("Thinker after fork, shm_id %d\n",*initial_shm_ptr);

    //Configure Signalhandling
    signal(SIGUSR1, signalHandler);
    printf("Thinker: registered handler SIGUSR1\n");


    // avoids orphan and zombie process, wait for child to die
    while(wait(NULL) > 0){
        //empty
    }

    //delete shared memory segment for Connector and Thinker
    clearShm(initial_shm_id);
    clearShm(*initial_shm_ptr);

    return 0;
}

