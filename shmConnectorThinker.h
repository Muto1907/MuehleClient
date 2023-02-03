#ifndef shmConnectorThinker_h
#define shmConnectorThinker_h

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>


typedef struct {

    char gameName[32];
    int myPlayerNumber;
    int enemyPlayerNumber;
    int countPlayer;
    pid_t idThinker;
    pid_t idConnector;
    bool flagProvideMove;
    int piecesToBeCaptured;

} GAMEINFO;

typedef struct
{
	int playerNum;
    int piecenum;
    char pos[2];
} PIECEINFO;

typedef struct {

    int playerNumber;
    char playerName[256];
    bool ready;
    bool isWinner;
    PIECEINFO piece[9];


} PLAYERINFO;

int createShm(GAMEINFO *gameInfo);
void *attachShm(int shm_id);
void clearShm(int shm_id);

#endif
