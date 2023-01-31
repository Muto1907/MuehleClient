#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "jumpPhase.h"
#include "thinking.h"
#include "shmConnectorThinker.h"
#include "errorHandling.h"

//initialising return string
char jumpSeq[1024];

int perturb(int i) {
    return 3*i+1;
}

char *jump(PLAYERINFO *currentPlayer) {
    time_t now = time(NULL);
    srand(now);
    int iter = 0;

    while(true) { //try different piece until piece is not yet captured
        memset(jumpSeq, 0, 1024);
        strcpy(jumpSeq, "PLAY ");

        //choosing a random piece from currentPlayer
        int randPiece = (rand()+perturb(iter)) %9;
        printf("randPiece value: %d\n", randPiece);
        PIECEINFO currentPiece = currentPlayer->piece[randPiece];

        if(strcmp(currentPiece.pos, "A") != 0 && strcmp(currentPiece.pos, "C") != 0) {
            while(true){ //try different positions on board until free
                int ring = (rand()+perturb(iter)) %3;
                int spot = (rand()+perturb(iter)) %8;
                if(isFreeBoardArr(ring, spot)) {
                    strcat(jumpSeq, currentPiece.pos);
                    strcat(jumpSeq, ":");
                    strcat(jumpSeq, remapCoordinates(ring, spot));
                    strcat(jumpSeq, "\n");
                    return jumpSeq;
                }
                iter++;
            }
        }
        iter++;
    }
    errFunctionFailed("jump");
    return "DON'T JUMP";
}