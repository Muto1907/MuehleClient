CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c performConnection.o errorHandling.o paramConfig.o

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

clean:
	rm -f *.o ./sysprak-client