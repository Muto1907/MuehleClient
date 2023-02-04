#ifndef thinking_h
#define thinking_h

void think(void* ptr_thinker, int tc_pipe[]); 

void dumpGameCurrent(PLAYERINFO**, GAMEINFO*);

int* mapCoord(PIECEINFO);
char* remapCoordinates(int, int);

bool isFree(char*);
bool isFreeBoardArr(int, int);

int getPlayernumberForPiece(int, int);

int countPieces(PLAYERINFO*, GAMEINFO*);
#endif
