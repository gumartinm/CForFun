all: locks

locks: locks.c
	mkdir -p bin/
	gcc -Wall -g -o bin/locks locks.c -lpthread

clean:
	rm -f locks
