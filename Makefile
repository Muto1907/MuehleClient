CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c

all: sysprak-client

sysprak-client: ${SOURCE} errorHandling.o 
	$(CC) $(FLAGS) -o $@ ${SOURCE} errorHandling.o 

debug:	${SOURCE}
	$(CC) $(FLAGS) -o $@ ${SOURCE} -g

errorHandling.o : errorHandling.c errorHandling.h
	$(CC) $(FLAGS) -c errorHandling.c  

clean:
	rm -f *.o ./sysprak-client