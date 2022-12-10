#ifndef performConnection_h
#define performConnection_h

#include "paramConfig.h"
#include "shmConnectorThinker.h"

void getServermsg(int fileDescriptor);
void sendMsgToServer(int fileDescriptor, char* msgInput);
int performConnection (int fileDescriptor, char* gameID, PARAM_CONFIG_T* cfg);

GAMEINFO* setParam();
//caller has to free the memory on its own using free()

#endif