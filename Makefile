CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c

all: sysprak-client

sysprak-client: ${SOURCE} errorHandling.h PerformConnection.h paramConfig.h errorHandling.o PerformConnection.o paramConfig.o
	$(CC) $(FLAGS) -o $@ ${SOURCE} errorHandling.o PerformConnection.o paramConfig.o

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