#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "shmConnectorThinker.h"
#include "performConnection.h"

#define EXIT_ERROR (-1)

int createShm(GAMEINFO *gameInfo){

    int shm_id;

    if((shm_id = shmget(IPC_PRIVATE, sizeof(GAMEINFO)+(gameInfo->countPlayer)*sizeof(PLAYERINFO)+(gameInfo->countPlayer)*gameInfo->piecesCount*sizeof(PIECEINFO), IPC_CREAT | 0666)) == -1) {

        errFunctionFailed("shm creation");
        return EXIT_ERROR;
    }
    else {
        return shm_id;
    }  

}

void *attachShm(int shm_id){

    void *shm_address;
    //evtl. Flags setzen, falls Connector nur schreibenden und Thinker nur lesenden Zugriff benötigt
    if((shm_address = shmat(shm_id, NULL, 0)) == (void *) -1) {

        errFunctionFailed("shm attachment");
        exit(EXIT_ERROR);
    }
    else {
        return shm_address;
    } 
} 

void clearShm(int shm_id){
    if((shmctl(shm_id, IPC_RMID, NULL)) == -1){

        errFunctionFailed("shm deletion");
    }  
} 
