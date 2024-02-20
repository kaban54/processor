CFLAGS += -Wall -DNDEBUG
CC = g++
OBJDIR = obj/
SRCDIR = src/

all: asm proc run

prepare:
	mkdir obj

asm: obj/asm.o obj/txtfuncs.o
	$(CC) -o asm obj/asm.o obj/txtfuncs.o $(CFLAGS)

proc: obj/proc.o obj/stack.o
	$(CC) -o proc obj/proc.o obj/stack.o $(CFLAGS)

run: src/run.cpp
	$(CC) -o run src/run.cpp $(CFLAGS)

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm obj/*.o -f
	clear
