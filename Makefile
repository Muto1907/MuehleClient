CC = /usr/bin/gcc
FLAGS = -Wall -Werror
SOURCE = main.c

all: sysprak-client

sysprak-client: ${SOURCE}
	$(CC) $(FLAGS) -o $@ ${SOURCE}

debug:	${SOURCE}
	$(CC) $(FLAGS) -o $@  ${SOURCE} -g

clean:
	rm -f *.o ./sysprak-client