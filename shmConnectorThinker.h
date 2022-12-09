#ifndef shmConnectorThinker
#define shmConnectorThinker

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <bool.h>


typedef struct {

    char *gameName;
    int myPlayerNumber;
    int countPlayer;
    pid_t idThinker;
    pid_t idConnector;

} GAMEINFO;

typedef struct {

    int myPlayerNumber;
    char *playerName;
    bool ready;

} PLAYERINFO;

GAMEINFO* getGameInfo();
PLAYERINFO* getPlayerInfo();
int createShm();

#endif