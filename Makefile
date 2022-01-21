# makefile for Virtual Memeory Unit (MMU)
#
# usage: make mmu 

CC=gcc
CFLAGS=-Wall

clean:
	rm -rf *.o
	rm -rf mmu


	
mmu: mmu.o
	$(CC) $(CFLAGS) -o mmu mmu.o

