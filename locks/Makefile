all: flocks fcntllocks

flocks: flocks.c
	gcc -Wall -g -o flocks flocks.c -lpthread

fcntllocks: fcntllocks.c
	gcc -Wall -g -o fcntllocks fcntllocks.c -lpthread

clean:
	rm -f flocks
	rm -f fcntllocks
