#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "errorHandling.h"
#include "shmConnectorThinker.h"
#include "performConnection.h"

#define EXIT_ERROR (-1)

int shm_id;
void *shm_address;
GAMEINFO gameInfo;

if((shm_id = shmget(IPC_PRIVATE, sizeof(GAMEINFO)+gameInfo.countPlayer*sizeof(PLAYERINFO), IPC_CREAT | IPC_EXCL)) == -1) {

    errFunctionFailed("shm creation");
    return EXIT_ERROR;

}


//evtl. Flags setzen, falls Connector nur schreibenden und Thinker nurlesenden Zugriff benötigt
if((shm_address = shmat(shm_id, NULL, 0))==-1) {

    errFunctionFailed("shm attachment");
    return EXIT_ERROR;
}
