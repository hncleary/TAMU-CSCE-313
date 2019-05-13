# makefile

CFLAGS=-g -std=c++11

all: unix

main.o : main.cpp
	g++ $(CFLAGS) -c main.cpp

unix: main.o
	g++ $(CFLAGS) -o unix main.o

clean:
	rm -f *.o unix
