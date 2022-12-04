#ifndef PerformConnection
#define PerformConnection

void getServermsg(int fileDescriptor);
void sendMsgToServer(int fileDescriptor, char* msgInput);
int performConnection (int fileDescrptor, char* gameID);