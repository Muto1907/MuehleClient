CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c

all: sysprak-client

sysprak-client: ${SOURCE} errorHandling.o PerformConnection.o
	$(CC) $(FLAGS) -o $@ ${SOURCE} errorHandling.o PerformConnection.o

debug:	${SOURCE}
	$(CC) $(FLAGS) -o $@ ${SOURCE} -g

errorHandling.o : errorHandling.c errorHandling.h
	$(CC) $(FLAGS) -c errorHandling.c  

PerformConnection.o : PerformConnection.c PerformConnection.h
	$(CC) $(FLAGS) -c PerformConnection.c  

clean:
	rm -f *.o ./sysprak-client