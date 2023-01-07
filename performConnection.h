#ifndef performConnection_h
#define performConnection_h

#include "paramConfig.h"
#include "shmConnectorThinker.h"

//exposed to be used in main.c
extern char serverMsg[];


void getServermsg(int fileDescriptor);
void sendMsgToServer(int fileDescriptor, char* msgInput);
int performConnection (int fileDescriptor, char* gameID, PARAM_CONFIG_T* cfg, int *initial_shm);
char** serverMsgToLines(char* serverMsg, char** linesOfServermsg);

GAMEINFO* setParam();
//caller has to free the memory on its own using free()
PLAYERINFO* setMyPlayerParam();
PLAYERINFO* setEnemyPlayerParam();


#endif