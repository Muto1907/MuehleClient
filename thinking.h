#ifndef thinking_h
#define thinking_h

void think(void* ptr_thinker, int tc_pipe[]); 

void dumpGameCurrent(PLAYERINFO*, GAMEINFO*);

void mapCoord(PIECEINFO*);

bool isFree(char*);
#endif
