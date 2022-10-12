CFLAGS += -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE -Wsign-compare
CC = g++

all: asm.exe proc.exe


asm.exe: asm.o
	$(CC) -o asm.exe asm.o $(CFLAGS)

proc.exe: proc.o stack.o
	$(CC) -o proc.exe proc.o stack.o $(CFLAGS)

asm.o: asm.cpp 
	$(CC) -o asm.o asm.cpp -c $(CFLAGS)

proc.o: proc.cpp 
	$(CC) -o proc.o proc.cpp -c $(CFLAGS)

stack.o: stack.cpp
	$(CC) -o stack.o stack.cpp -c $(CFLAGS)

run.exe: run.cpp
	$(CC) -o run.exe run.cpp $(CFLAGS)


clean:
	rm *.o
	clear
	
.PHONY: clean