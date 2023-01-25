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
char currentPosition[24];

char *makeAMove( PLAYERINFO *currentPlayer) {

    time_t now = time(NULL);
    srand(now);

    while(true) { //try different pieces until some neighbouring place for current piece is free and function terminates
        memset(moveSeq, 0, 1024);
        strcpy(moveSeq, "PLAY ");

        //choosing a random piece from currentPlayer
        int randPiece = rand() % 9;
        printf("randPiece value: %d\n", randPiece);

        //check if piece is already captured
        //if so try a different random piece number
        while(strcmp(currentPlayer->piece[randPiece].pos, "C") == 0) {
            printf("currentPiece is captured.\n");
            randPiece = rand() % 9;
        }
        PIECEINFO currentPiece = currentPlayer->piece[randPiece];
        strcat(moveSeq, currentPiece.pos);
        strcat(moveSeq, ":");

        //get corresponding board position for the currentPiece
        int* pos = mapCoord(currentPiece);
        int coordR = pos[0];
        int coordS = pos[1];

        /* forall neighbouring places: check if free
        if so: move there and leave function
        else: continue searching */

        printf("Position von Spielstein %d, Position: %s, entspricht: %d%d\n", currentPiece.piecenum, currentPlayer->piece[randPiece].pos, coordR, coordS);

        //checking neighbours on same ring
        if(isFreeBoardArr((coordR %3), coordS-1 %8)) {
            strcat(moveSeq, remapCoordinates(coordR, coordS-1 %8));
            return moveSeq;
        }
        if(isFreeBoardArr(coordR, coordS+1 %8)) {
            strcat(moveSeq, remapCoordinates(coordR, coordS+1 %8));
            return moveSeq;
        }

        //checking neighbours on different rings 
        if(coordS % 2 == 1) { //at least one neighbour is on a different ring
            if(coordR == 0 || coordR == 1) {
                    if(isFreeBoardArr(coordR+1 % 3, coordS)) {
                        strcat(moveSeq, remapCoordinates(coordR+1 %3, coordS));
                        return moveSeq;
                    }
            }
            if(coordR == 1 || coordR == 2) {
                    if(isFreeBoardArr(coordR-1 % 3, coordS)) {
                        strcat(moveSeq, remapCoordinates(coordR-1 %3, coordS));
                        return moveSeq;
                    }
            }
        }
    }

    errFunctionFailed("makeAMove");
    return "DON'T PLAY";
}