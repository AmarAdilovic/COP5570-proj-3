CC=gcc
CFLAG=-ansi -Wall -pedantic

myserver: myserver.o
	$(CC) $(CFLAG) myserver.o -o myserver
%.o: %.c myserver.h
	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o myserver