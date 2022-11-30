#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define GAMEKINDNAME "NMMorris"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"

void printAnweisung(){
    printf("-g <GAME-ID> 13-stellige Game-ID\n");
    printf("-p gewünschte Spielernummer\n");
}


int main(int argc,char**argv){

 long long game_id=0;
    unsigned int playernumber=0;
    int option;

    while((option = getopt(argc, argv, "g:p:")) != -1){
        switch(option){
            case 'g':
            game_id = atoll(optarg);
            break;
            case 'p':
            playernumber = atoi(optarg);
            break;
            default:
            printAnweisung();
            return 0;
        }
    }

    // Testet, ob Game-ID 13 stellen hat
    int counter = 1;
    long long zahl = game_id;
    while(zahl > 9){
        zahl /= 10;
        counter++;
    }

    if(counter != 13){
        printf("Ungültige Game-ID.\n");
        printAnweisung();
        return -1;
    }

    //Testet ob Spielernummer 1 oder 2 ist 
    if(playernumber < 1 || playernumber > 2){
        printf("Ungültige Spielernummer.\n");
        printAnweisung();
        return -1;
    }

    return 0;
}