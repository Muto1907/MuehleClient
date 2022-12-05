#ifndef performConnection
#define performConnection

void getServermsg(int fileDescriptor);
void sendMsgToServer(int fileDescriptor, char* msgInput);
int performConnection (int fileDescrptor, char* gameID);

#endif