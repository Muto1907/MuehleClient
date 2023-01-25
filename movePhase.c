#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "movePhase.h"
#include "thinking.h"
#include "shmConnectorThinker.h"
#include "errorHandling.h"

//initialising return string
char moveSeq[1024];

char *makeAMove(PIECEINFO *board, PLAYERINFO *currentPlayer) {

    while(true) { //try different pieces until some neighbouring place for current piece is free and function terminates
        memset(moveSeq, 0, 1024);
        strcpy(moveSeq, "PLAY ");

        //choosing a random piece from currentPlayer
        time_t now = time(NULL);
        srand(now);
        int randPiece = rand() % 8;

        //check if piece is already captured
        //if so try a different random piece number
        while(strcmp(currentPlayer->piece[randPiece].pos, "C ")) {
            randPiece = rand() % 8;
        }
        PIECEINFO currentPiece = currentPlayer->piece[randPiece];
        strcat(moveSeq, currentPiece.pos+':');

        //get corresponding board position for the currentPiece
        int* pos = mapCoord(currentPiece);
        int coordR = pos[0];
        int coordS = pos[1];

        /* forall neighbouring places: check if free
        if so: move there and leave function
        else: continue searching */

        //checking neighbours on same ring
        if(isFree(remapCoordinates(coordR, coordS-1 %8))) {
            strcat(moveSeq, remapCoordinates(coordR, coordS-1 %8)+'\0');
            return moveSeq;
        }
        if(isFree(remapCoordinates(coordR, coordS+1 %8))) {
            strcat(moveSeq, remapCoordinates(coordR, coordS+1 %8)+'\0');
            return moveSeq;
        }

        //checking neighbours on different rings 
        if(coordS % 2 == 1) { //at least one neighbour is on a different ring
            if(coordR == 1 || coordR == 2) {
                    if(isFree(remapCoordinates(coordR+1, coordS))) {
                        strcat(moveSeq, remapCoordinates(coordR+1 %3, coordS)+'\0');
                        return moveSeq;
                    }
            }
            if(coordR == 2 || coordR == 3) {
                    if(isFree(remapCoordinates(coordR-1, coordS))) {
                        strcat(moveSeq, remapCoordinates(coordR-1 %3, coordS)+'\0');
                        return moveSeq;
                    }
            }
        }
    }
    //TODO return error message
//for Test purpose
errFunctionFailed("makeAMove");
return "DON'T PLAY";
}