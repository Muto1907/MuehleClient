#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shmConnectorThinker.h"
#include "setPhase.h"
#include "thinking.h"

char buf[1024];
//char result[6];
char coordinates[12];

/* int perturb(int i) {
    return 3*i+1;
} */

char* setPiece( PIECEINFO* piece){

    //memset(result, 0, 6);
    memset(buf, 0, 1024);
    strcpy(buf, "PLAY ");
    //initialize 2 random integers by using time so the ints vary on each run
    time_t now = time(NULL);
    srand(now); 
    int iter = 0;
    int positionR = rand() % 3;
    int positionS = rand() % 8;
    //test
    printf("Random numbers: %d and %d\n", positionR, positionS);
    //check if the position is free if not pick another random position
     while(!isFreeBoardArr(positionR, positionS)){

            positionR = (rand()+3*iter+1) % 3;
            positionS = (rand()+3*iter+1) % 8;
            iter++;
            //test
            printf("Changed numbers to %d and %d\n", positionR, positionS);
    }
    //bring the PLAY command into protocoll-format
    strcpy(coordinates, remapCoordinates(positionR, positionS));
    strcat(buf, coordinates);
    strcat(buf, "\n");
    

    return buf;
}
