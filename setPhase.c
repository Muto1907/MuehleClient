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

char* setPiece( PIECEINFO* piece){

    //memset(result, 0, 6);
    memset(buf, 0, 1024);
    strcpy(buf, "PLAY ");
    //initialize 2 random integers by using time so the ints vary on each run
    time_t now = time(NULL);
    srand(now); 
    int positionR = rand() % 3;
    int positionS = rand() % 8;
    //test
    printf("Random numbers: %d and %d\n", positionR, positionS);
    //check if the position is free if not pick another random position
     while(!isFreeBoardArr(positionR, positionS)){

            positionR = rand() % 3;
            positionS = rand() % 8;
            //test
            printf("Changed numbers to %d and %d\n", positionR, positionS);
    }
    //bring the PLAY command into protocoll-format
    strcpy(coordinates, remapCoordinates(positionR, positionS));
    strcat(buf, coordinates);
    strcat(buf, "\n");
    

    return buf;
}

//map the two random integers into the corresponding position
/* char* remapCoordinates(int first, int second){	

	switch(first)
	{
		case 0:
			result[0] = 'A';
			break;
		case 1:
			result[0] = 'B';
			break;
		case 2:
			result[0] = 'C';
			break;
		default:
			break;
	}
    result[1] = second + '0';
    //sprintf(result+1, "%d", second);
    result[2] = '\0';

    //printf("first: %c, and second: %c", result[0], result[1]);
    return result;
}
 */
