CC=gcc
CFLAG=-Wall -std=c99 -D_POSIX_C_SOURCE=200809L -pedantic -g

myserver: myserver.o mygame.o mycommands.o
	$(CC) $(CFLAG) myserver.o mygame.o mycommands.o -o myserver
%.o: %.c myserver.h
	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o myserver