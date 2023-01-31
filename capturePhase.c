#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "capturePhase.h"
#include "thinking.h"
#include "shmConnectorThinker.h"
#include "errorHandling.h"

//initialising return string
char captureSeq[1024];
bool partOfMill = false;

int perturb(int i) {
    return 3*i+1;
}

char *captureAPiece(PLAYERINFO *enemyPlayer) {
    time_t now = time(NULL);
    srand(now);
    int iter = 0;

    while(true) { //try different pieces until current piece is not part of a mill
        memset(captureSeq, 0, 1024);
        strcpy(captureSeq, "PLAY ");

        //choosing a random piece from enemyPlayer
        int randPiece = (rand()+perturb(iter)) %9;
        iter++;
        printf("randPiece value: %d\n", randPiece);
        PIECEINFO currentPiece = enemyPlayer->piece[randPiece];

        //get corresponding board position for the currentPiece
        int* pos = mapCoord(currentPiece);
        int coordR = pos[0];
        int coordS = pos[1];

        printf("Position von Spielstein %d, Position: %s, entspricht: %d%d\n", currentPiece.piecenum, enemyPlayer->piece[randPiece].pos, coordR, coordS);

        //check if chosen piece is part of a mill and cannot be captured
        if(strcmp(currentPiece.pos, "A") != 0 && strcmp(currentPiece.pos, "C") != 0) {
            if(coordS % 2 == 1) { //at least one neighbour is on a different ring)
                if(isFreeBoardArr(coordR, (coordS-1) %8) && getPlayernumberForPiece(coordR, (coordS-1) %8) == enemyPlayer->playerNumber 
                && isFreeBoardArr(coordR, (coordS+1) %8) && getPlayernumberForPiece(coordR, (coordS+1) %8) == enemyPlayer->playerNumber) { //mill on same ring
                    partOfMill = true;
                }
                else if(isFreeBoardArr((coordR+1) %3, coordS) && getPlayernumberForPiece((coordR+1) %3, coordS) == enemyPlayer->playerNumber 
                && isFreeBoardArr((coordR+2) %3, coordS) && getPlayernumberForPiece((coordR+2) %3, coordS) == enemyPlayer->playerNumber) { //mill on spanning all three rings
                    partOfMill = true;
                }
            }
            else { //all neighbours are on the same ring
                if(isFreeBoardArr(coordR, (coordS-2) %8) && getPlayernumberForPiece(coordR, (coordS-2) %8) == enemyPlayer->playerNumber 
                && isFreeBoardArr(coordR, (coordS-1) %8) && getPlayernumberForPiece(coordR, (coordS-1) %8) == enemyPlayer->playerNumber) { //mill "to the left" of currentPiece
                    partOfMill = true;
                }
                else if(isFreeBoardArr(coordR, (coordS+2) %8) && getPlayernumberForPiece(coordR, (coordS+2) %8) == enemyPlayer->playerNumber 
                && isFreeBoardArr(coordR, (coordS+1) %8) && getPlayernumberForPiece(coordR, (coordS+1) %8) == enemyPlayer->playerNumber) { //mill "to the right" of currentPiece
                    partOfMill = true;
                }
            }
        }
        else continue;

        //append position of currentPiece to captureSeq and terminate function if piece is not part of a mill
        if(!partOfMill) {
            strcat(captureSeq, remapCoordinates(coordR, coordS));
            strcat(captureSeq, "\n");
            return captureSeq;
        }
    }

    errFunctionFailed("captureAPiece");
    return "DON'T PLAY";
}