#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"

void printAnweisung(){
    printf("-g <GAME-ID> 13-stellige Game-ID\n");
    printf("-p gewünschte Spielernummer\n");
}


int main(int argc,char**argv){

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

    if(strlen(game_id) != 15){
        printf("Ungültige Game-ID.\n");
        printAnweisung();
        return -1;
    }

    //Test, if playernumber is 1 or 2
    if(strcmp(playernumber, "1") != 0|| strcmp(playernumber, "2") != 0){
        printf("Ungültige Spielernummer.\n");
        printAnweisung();
        return -1;
    }


    //Preparing connection to server "sysprak.priv.lab.nm.ifi.lmu.de"
    int socketfd = getSocketDescriptorAndConnect();
    //TODO error handling for socketfd == -1

    performConnection(socketfd, game_id);

    return 0;
}

/*getSocketDescriptorAndConnect creates a socket and tries to connect to the LMU server.
* returns socket file descriptor socketfd upon success, otherwise -1
*/
int getSocketDescriptorAndConnect(){
    int socketfd;

    struct sockaddr_in address;
    memset(&addr, 0, sizeof(addr)); //Filling struct values with 0
    address.sin_family = AF_INET;
    address.sin_port = htons(PORTNUMBER);

    //Retrieving the server address using getaddrinfo()
    struct addrinfo hints, *res, *result;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET; //using IPv4 addresses
    hints.ai_socktype = SOCK_STREAM; //using TCP connection
    hints.ai_protocol = IPPROTO_TCP; //using TCP protocol

    if(getaddrinfo(HOSTNAME, NULL, &hints, &result) != 0) {
        perror("getaddrinfo"); //TODO Error handling
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

        if((socketfd = socket(AF_INET, SOCK_STREAM, 0))==-1) {
            continue;
        }

        address.sin_addr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;

        if(connect(socketfd, (struct sockaddr *) &address, sizeof(address)) != -1) {
            break;
        }

        close(socketfd);
    }
    
    freeaddrinfo(result); //no longer needed since IP-address of server has been resolved

    //All connection attempts failed, i.e. res reached the end of the list of address structures
    if(NULL == res) {
        printf("Could not connect to server %s.\n", HOSTNAME);
        //TODO Error handling
        return -1;
    }

    return socketfd;
}