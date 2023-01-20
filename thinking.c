#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "errorHandling.h"
#include "shmConnectorThinker.h"
#include "thinking.h"

const PIECEINFO dummy = {-1, -1, "NN"};
PIECEINFO boardArr[3][8] = {{dummy}};

void think(void* ptr_thinker, int tc_pipe[])
{
    printf("thinking 2.0...\n");
    

    GAMEINFO* game = (GAMEINFO*) ptr_thinker;
    PLAYERINFO *player = (PLAYERINFO *) (game+1);
    if(game->flagProvideMove)
    {
        dumpGameCurrent(player, game);
        //let the thinker think and send it to the connector
        char buff[] = "A1\n";
        // thinker writes to pipe
        if(write(tc_pipe[1], buff, strlen(buff) + 1) == -1){
            perror("thinker can't write.\n");
            exit(0);
        }




        game->flagProvideMove = false;
		// TODO : KI
	}
}

#if 0
static void setTestPieces (PLAYERINFO* player)
{
    int i;
    for (i=0;i<9;i++)
    {
        player[0].piece[i].piecenum = i;
        player[0].piece[i].pos[0] = 'A';
        player[0].piece[i].pos[1] = '0' + i;
        player[1].piece[i].piecenum = i;
        player[1].piece[i].pos[0] = 'C';
        player[1].piece[i].pos[1] = '0' + i;
    }
}
#endif

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
    printf("dumpGameCurrent: Gameinfo %p, playerinfo %p\n",game,player);

    //setTestPieces(player); //testing
    int numPlayer = game->countPlayer;

    char    board[13][28];
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

void mapCoord(PIECEINFO* piece)
{
	int coordR = 0;
	
	switch(piece->pos[0])
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
	
	int coordS = (int)(piece->pos[1]) % 8;
	
	boardArr[coordR][coordS] = *piece;
}

bool isFree(char* pos)
{
	bool free = false;
	if(strcmp(pos, dummy.pos))
		free = true;
	return free;
}
