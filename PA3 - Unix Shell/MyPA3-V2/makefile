# makefile

CFLAGS=-g -std=c++11

all: shell

main.o : main.cpp
	g++ $(CFLAGS) -c main.cpp

shell: main.o
	g++ $(CFLAGS) -o shell main.o

clean:
	rm -f *.o shell
