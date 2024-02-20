CFLAGS += -Wall
CC = g++

all: asm.exe proc.exe


asm.exe: asm.o txtfuncs.o
	$(CC) -o asm.exe asm.o txtfuncs.o $(CFLAGS)

proc.exe: proc.o stack.o
	$(CC) -o proc.exe proc.o stack.o $(CFLAGS)

asm.o: asm.cpp
	$(CC) -o asm.o asm.cpp -c $(CFLAGS)

txtfuncs.o: txtfuncs.cpp 
	$(CC) -o txtfuncs.o txtfuncs.cpp -c $(CFLAGS)

proc.o: proc.cpp 
	$(CC) -o proc.o proc.cpp -c $(CFLAGS)

stack.o: stack.cpp
	$(CC) -o stack.o stack.cpp -c $(CFLAGS)

run.exe: run.cpp
	$(CC) -o run.exe run.cpp $(CFLAGS)


clean:
	rm *.o
	clear
	
.PHONY: clean asm.exe proc.exe