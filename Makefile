CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c performConnection.o errorHandling.o paramConfig.o shmConnectorThinker.o thinking.o

all: sysprak-client

sysprak-client: ${SOURCE} 
	$(CC) $(FLAGS) -o $@ ${SOURCE} 

debug:	${SOURCE}
	$(CC) $(FLAGS) -o $@ ${SOURCE} -g

errorHandling.o : errorHandling.c errorHandling.h
	$(CC) $(FLAGS) -c errorHandling.c  

performConnection.o : performConnection.c performConnection.h
	$(CC) $(FLAGS) -c performConnection.c  

paramConfig.o : paramConfig.c paramConfig.h
	$(CC) $(FLAGS) -c paramConfig.c  

shmConnectorThinker.o : shmConnectorThinker.c shmConnectorThinker.h
	$(CC) $(FLAGS) -c shmConnectorThinker.c

thinking.o : thinking.c thinking.h
	$(CC) $(FLAGS) -c thinking.c

movePhase.o : movePhase.c movePhase.h
	$$(CC) $(FLAGS) -c movePhase.c

clean:
	rm -f *.o ./sysprak-client