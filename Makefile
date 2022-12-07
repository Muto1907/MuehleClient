CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c performConnection.c

all: sysprak-client

sysprak-client: ${SOURCE} 
	$(CC) $(FLAGS) -o $@ ${SOURCE}

debug:	${SOURCE}
	$(CC) $(FLAGS) -o $@ ${SOURCE} -g

errorHandling.o : errorHandling.c errorHandling.h
	$(CC) $(FLAGS) -c errorHandling.c  

PerformConnection.o : PerformConnection.c PerformConnection.h
	$(CC) $(FLAGS) -c PerformConnection.c  

paramConfig.o : paramConfig.c paramConfig.h
	$(CC) $(FLAGS) -c paramConfig.c  

clean:
	rm -f *.o ./sysprak-client