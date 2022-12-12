#ifndef shmConnectorThinker_h
#define shmConnectorThinker_h

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>


typedef struct {

    char gameName[32];
    int myPlayerNumber;
    int countPlayer;
    pid_t idThinker;
    pid_t idConnector;

} GAMEINFO;

typedef struct {

    int myPlayerNumber;
    char playerName[256];
    bool ready;

} PLAYERINFO;

/* GAMEINFO* getGameInfo();
PLAYERINFO* getPlayerInfo(); */
int createShm(GAMEINFO *gameInfo);
void *attachShm(int shm_id);
void clearShm(int shm_id);

#endif