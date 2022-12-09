#ifndef performConnection

void getServermsg(int fileDescriptor);
void sendMsgToServer(int fileDescriptor, char* msgInput);
int performConnection (int fileDescriptor, char* gameID);

GAMEINFO* setParam();

#endif
