# makefile

all: memtest

Ackerman.o: Ackerman.cpp 
	g++ -c -g Ackerman.cpp

BuddyAllocator.o : BuddyAllocator.cpp
	g++ -c -g BuddyAllocator.cpp

Main.o : Main.cpp
	g++ -c -g Main.cpp

memtest: Main.o Ackerman.o BuddyAllocator.o
	g++ -o memtest Main.o Ackerman.o BuddyAllocator.o

clean:
	rm *.o