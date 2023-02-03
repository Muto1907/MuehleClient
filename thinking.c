#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "errorHandling.h"
#include "shmConnectorThinker.h"
#include "thinking.h"
#include "setPhase.h"
#include "movePhase.h"
#include "capturePhase.h"
#include "jumpPhase.h"

#define length(x) (sizeof(x) / sizeof(*(x)))



const PIECEINFO dummy = {-1, -1, "N"};
PIECEINFO boardArr[3][8];
char result[6];
int resultingPos[2];
bool flagPrt= true;
char buff[1024];
int iter = 0;


int countPieces(PLAYERINFO player) {
    int counter = 0;
    for(int i=0; i < length(player.piece); i++) {
        if(strcmp(player.piece[i].pos, "A") != 0 && strcmp(player.piece[i].pos, "C") != 0)
            counter++;
    }

    return counter;
}

void think(void* ptr_thinker, int tc_pipe[])
{
    memset(buff, 0, 1024);
    GAMEINFO* game = (GAMEINFO*) ptr_thinker;
    PLAYERINFO *player = (PLAYERINFO *) (game+1);

    int rows = length(boardArr);
    int columns = length(boardArr[0]);

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < columns; j++){
            boardArr[i][j] = dummy;
        }
    }

    if(game->flagProvideMove)
    {
        for(int i = 0; i < game->countPlayer; i++){
            for(int j = 0; j < length(player->piece); j++){
                mapCoord(player[i].piece[j]);
            }
        }
        dumpGameCurrent(player, game);
        printf("Thinker Pieces to be Captured shm: %d\n",game->piecesToBeCaptured);

        if(game->piecesToBeCaptured > 0){
            strcpy(buff, captureAPiece(&player[game->enemyPlayerNumber], iter));
            iter = (iter+1) %17;
        }
        //check if last piece is still Available. If not setPhase is over

        else if(strcmp(player[game->myPlayerNumber].piece[8].pos, "A") == 0){
            //setPhase begins here:
            strcpy(buff, setPiece (player[game->myPlayerNumber].piece, iter));
            iter = (iter+1) %17;

        }
        else if(countPieces(player[game->myPlayerNumber]) > 3){ //player has more than three pieces on board
            if (flagPrt)
			flagPrt = false;
            //MovePhase begins here:
            strcpy(buff, makeAMove(&player[game->myPlayerNumber], iter));
            iter = (iter+1) %17;

            printf("this is the command: %s\n", buff);
        }
        else {//player has three pieces on board
            //JumpPhase begins here:
            strcpy(buff, jump(&player[game->myPlayerNumber], iter));
            iter = (iter+1) %17;

            printf("Jumpcommand: %s\n", buff);
        }
            // thinker writes to pipe
        if(write(tc_pipe[1], buff, strlen(buff) + 1) == -1){
            perror("thinker can't write.\n");
            exit(0);
            }



        game->flagProvideMove = false;
	}
}

static char *boardtemplate[13] = 
{
" +-----------+-----------+ ",
" |           |           | ",
" |   +-------+-------+   | ",
" |   |       |       |   | ",
" |   |   +---+---+   |   | ",
" |   |   |       |   |   | ",
" +---+---+       +---+---+ ",
" |   |   |       |   |   | ",
" |   |   +---+---+   |   | ",
" |   |       |       |   | ",
" |   +-------+-------+   | ",
" |           |           | ",
" +-----------+-----------+ "
};

static int boardposX[24] = 
{
     0, 12, 24, 24, 24, 12,  0,  0, //A0 - A7
     4, 12, 20, 20, 20, 12,  4,  4, //B0 - B7
     8, 12, 16, 16, 16, 12,  8,  8  //C0 - C7
};

static int boardposY[24] = 
{
     0,  0,  0,  6, 12, 12, 12,  6, //A0 - A7
     2,  2,  2,  6, 10, 10, 10,  6, //B0 - B7
     4,  4,  4,  6,  8,  8,  8,  6  //C0 - C7
};

static int MapPosition(char* pos)
{
    int res = -1;
    if (pos[0] >= 'A' && pos[0] <= 'C' && pos[1] >= '0' && pos[1] <= '7')
    {
        res = (pos[0] - 'A') * 8 + (pos[1] - '0');
    }
    return res;
}

void dumpGameCurrent(PLAYERINFO* player, GAMEINFO* game)
{
    int numPlayer = game->countPlayer;

    char board[13][28];
    int i,j;
    for (i=0;i<13;i++)
        strcpy(board[i],boardtemplate[i]);

    for (i=0;i<numPlayer;i++)
        for (j=0;j<9;j++)
        {
            int pos = MapPosition(player[i].piece[j].pos);
            if (pos >= 0)
            {
                board[boardposY[pos]][boardposX[pos]  ] = '(';
                board[boardposY[pos]][boardposX[pos]+1] = '1' + i;
                board[boardposY[pos]][boardposX[pos]+2] = ')';
            }
        }
    for (i=0;i<13;i++)
        printf("%s\n",board[i]);
}

int *mapCoord(PIECEINFO piece)
{
	memset(resultingPos, -1, 8);

    if(strcmp(piece.pos, "C") != 0 && strcmp(piece.pos, "A") != 0) {
        
        int coordR = 0;
                
        switch(piece.pos[0])
        {
            case 'A':
                coordR = 0;
                break;
            case 'B':
                coordR = 1;
                break;
            case 'C':
                coordR = 2;
                break;
            default:
                break;
        }
        
        int coordS = ((int)(piece.pos[1]) - '0') % 8;
        
        boardArr[coordR][coordS] = piece;

        resultingPos[0] = coordR;
        resultingPos[1] = coordS;
    }
    
    return resultingPos;
}

char* remapCoordinates(int posR, int posS){	

    memset(result, 0, 6);

	switch(posR)
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
    //modulo operator in c doesnt account for negative numbers
    if(posS < 0){
        posS += 8;
    }
    result[1] = posS + '0';
    result[2] = '\0';

    return result;
}

bool isFree(char* pos)
{
	bool free = false;
	if(strcmp(pos, dummy.pos) == 0)
		free = true;
	return free;
}

bool isFreeBoardArr(int posR, int posS) {
    bool free = false;
    //modulo operator in c doesnt account for negative numbers
    if(posS < 0){
        posS += 8;
    }
    PIECEINFO currentPiece = boardArr[posR][posS];
    printf("isFreeBoardArr: position [%d][%d] with value: %s\n", posR, posS, currentPiece.pos);
	if(strcmp(currentPiece.pos, dummy.pos) == 0)
		free = true;
	return free;
}

int getPlayernumberForPiece(int posR, int posS) {
    PIECEINFO currentPiece = boardArr[posR][posS];
    return currentPiece.playerNum;
}
