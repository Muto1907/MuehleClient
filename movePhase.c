#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "movePhase.h"
#include "thinking.h"
#include "shmConnectorThinker.h"

char *makeAMove(PIECEINFO *board, PLAYERINFO *currentPlayer) {
    int perturbance = 0; //to perturb rand() in case it always returns the same value

    while(true) { //try different pieces until some neighbouring place for current piece is free and function terminates
        //initialising return string
        char *moveSeq = "";

        //choosing a random piece from currentPlayer
        int randPiece = rand() + perturbance % 9;

        //check if piece is already captured
        //if so try a different random piece number
        while(strcmp(currentPlayer->piece[randPiece].pos, "C ")) {
            randPiece = rand() + perturbance % 9;
        }
        PIECEINFO currentPiece = currentPlayer->piece[randPiece];
        moveSeq += currentPiece.pos;

        //get corresponding board position for the currentPiece
        int coordR = getMapCoord(&currentPiece);
        int coordS = (int)(currentPiece.pos[1]) % 8;

        /* forall neighbouring places: check if free
        if so: move there and leave function
        else: continue searching */

        //checking neighbours on same ring
        if(isFreeBoardArr({coordR, (coordS-1 %8)})) {
            moveSeq += ":" + getCoordR(coordR, (coordS-1 %8)) + getCoordS(coordR, (coordS-1 %8));
            return moveSeq;
        }
        if(isFreeBoardArr({coordR, (coordS+1 %8)})) {
            moveSeq += ":" + getCoordR(coordR, (coordS+1 %8)) + getCoordS(coordR, (coordS+1 %8));
            return moveSeq;
        }

        //checking neighbours on different rings 
        if(coordS % 2 == 1) { //at least one neighbour is on a different ring
            if(coordR == 1 || coordR == 2) {
                    if(isFreeBoardArr({coordR+1, coordS})) {
                        moveSeq += ":" + getPosition(coordR+1, coordS);
                        return moveSeq;
                    }
            }
            if(coordR == 2 || coordR == 3) {
                    if(isFreeBoardArr({coordR-1, coordS})) {
                        moveSeq += ":" + getPosition(coordR-1, coordS);
                        return moveSeq;
                    }
            }
        }

        perturbance ++;
    }
//for Test purpose
//return "1";
}