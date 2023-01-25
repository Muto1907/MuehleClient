#ifndef thinking_h
#define thinking_h

void think(void* ptr_thinker, int tc_pipe[]); 

void dumpGameCurrent(PLAYERINFO*, GAMEINFO*);

void mapCoord(PIECEINFO);

int getMapCoordRing(PIECEINFO*);
char getCoordR(int, int);
char getCoordS(int, int);

bool isFree(char*);
bool isFreeBoardArr(int*);

#endif
