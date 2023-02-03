#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "movePhase.h"
#include "thinking.h"
#include "shmConnectorThinker.h"
#include "errorHandling.h"

// initialising return string
char moveSeq[1024];
char currentPosition[24];

char *makeAMove(PLAYERINFO *currentPlayer, int iter)
{

    time_t now = time(NULL);
    srand(now);

    int randPiece = (rand() + 3 * iter + 1) % 9;

    while (true)
    { // try different pieces until some neighbouring place for current piece is free and function terminates
        memset(moveSeq, 0, 1024);
        strcpy(moveSeq, "PLAY ");

        // choosing a random piece from currentPlayer
        randPiece = (rand() + 3 * iter + 1) % 9; // perturbance makes different piece choice for each iteration more likely
        iter++;

        // check if piece is already captured or available (shouldn't be the case at this stage)
        // if so try a different random piece number
        while (strcmp(currentPlayer->piece[randPiece].pos, "C") == 0 || strcmp(currentPlayer->piece[randPiece].pos, "A") == 0)
        {
            randPiece = (rand() + 3 * iter + 1) % 9;
            iter++;
        }
        PIECEINFO currentPiece = currentPlayer->piece[randPiece];
        strcat(moveSeq, currentPiece.pos);
        strcat(moveSeq, ":");

        // get corresponding board position for the currentPiece
        int *pos = mapCoord(currentPiece);
        int coordR = pos[0];
        int coordS = pos[1];

        /* forall neighbouring places: check if free
        if so: move there and leave function
        else: continue searching */

        int order_lr = 0;   // determines in which order neighbours (left/right) on same ring are checked
        int order_ring = 0; // determines in which order neigbours (same/different ring) are checked
        if (randPiece % 2 == 0)
        {
            order_lr = 1;
            if (coordS % 2 == 1) // at least one neighbour is on a different ring)
                order_ring = 1;
        }
        else
        {
            order_lr = -1;
            if (coordS % 2 == 1) // at least one neighbour is on a different ring)
                order_ring = -1;
        }

        // checking neighbours on different rings
        if (order_ring == 1)
        {
            if (coordR == 0 || coordR == 1)
            {
                if (isFreeBoardArr((coordR + 1) % 3, coordS))
                {
                    strcat(moveSeq, remapCoordinates((coordR + 1) % 3, coordS));
                    strcat(moveSeq, "\n");
                    return moveSeq;
                }
            }
            if (coordR == 1 || coordR == 2)
            {
                if (isFreeBoardArr((coordR - 1) % 3, coordS))
                {
                    strcat(moveSeq, remapCoordinates((coordR - 1) % 3, coordS));
                    strcat(moveSeq, "\n");
                    return moveSeq;
                }
            }
        }

        // checking neighbours on same ring
        if (isFreeBoardArr(coordR % 3, (coordS - order_lr) % 8))
        {
            strcat(moveSeq, remapCoordinates(coordR, ((coordS - order_lr) % 8)));
            strcat(moveSeq, "\n");
            return moveSeq;
        }
        if (isFreeBoardArr(coordR, (coordS + order_lr) % 8))
        {
            strcat(moveSeq, remapCoordinates(coordR, (coordS + order_lr) % 8));
            strcat(moveSeq, "\n");
            return moveSeq;
        }

        // checking neighbours on different rings
        if (order_ring == -1)
        {
            if (coordR == 0 || coordR == 1)
            {
                if (isFreeBoardArr((coordR + 1) % 3, coordS))
                {
                    strcat(moveSeq, remapCoordinates((coordR + 1) % 3, coordS));
                    strcat(moveSeq, "\n");
                    return moveSeq;
                }
            }
            if (coordR == 1 || coordR == 2)
            {
                if (isFreeBoardArr((coordR - 1) % 3, coordS))
                {
                    strcat(moveSeq, remapCoordinates((coordR - 1) % 3, coordS));
                    strcat(moveSeq, "\n");
                    return moveSeq;
                }
            }
        }
    }

    errFunctionFailed("makeAMove");
    return "DON'T PLAY";
}