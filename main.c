#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include "errorHandling.h"
#include "PerformConnection.h"

#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"

void printAnweisung(){
    printf("-g <GAME-ID> 13-stellige Game-ID\n");
    printf("-p gewünschte Spielernummer\n");
}

/*getSocketDescriptorAndConnect creates a socket and tries to connect to the LMU server.
* returns socket file descriptor socketfd upon success, otherwise -1
*/
int getSocketDescriptorAndConnect(){
    int socketfd;
    char portnb[4*sizeof(int)];
    snprintf(portnb, sizeof(portnb), "%d", PORTNUMBER); //converting int PORTNUMBER to char-array portnb

    //Retrieving the server address using getaddrinfo()
    struct addrinfo hints, *res, *result;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC; //using IPv4 or IPv6 addresses
    hints.ai_socktype = SOCK_STREAM; //using TCP connection
    hints.ai_protocol = IPPROTO_TCP; //using TCP protocol

    if(getaddrinfo(HOSTNAME, portnb, &hints, &result) != 0) {
        errWithHost ("getaddrinfo", HOSTNAME);
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
        res = res->ai_next;

        if((socketfd = socket(res->ai_family, SOCK_STREAM, 0))==-1) {
            continue;
        }

        if(connect(socketfd, res->ai_addr, res->ai_addrlen) != -1) {
            break;
        }

        close(socketfd);
    }
    // TODO: segmentation fault when running from home (even with vpn)


    freeaddrinfo(result); //no longer needed since IP-address of server has been resolved

    //All connection attempts failed, i.e. res reached the end of the list of address structures
    if(NULL == res) {
        errWithHost("Connect to server", HOSTNAME);
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

    while((option = getopt(argc, argv, "g:p:")) != -1){
        switch(option){
            case 'g':
            strcpy(game_id, optarg);
            break;
            case 'p':
            strcpy(playernumber, optarg);
            break;
            default:
            printAnweisung();
            return 0;
        }
    }

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


    //Preparing connection to server "sysprak.priv.lab.nm.ifi.lmu.de"
    int socketfd = getSocketDescriptorAndConnect();
    printf("socket fd: %d\n", socketfd); //for testing only!!
    if (socketfd == -1)
        errFunctionFailed ("getSocketDescriptorAndConnect");
    else
        performConnection(socketfd, game_id);

    return 0;
}

