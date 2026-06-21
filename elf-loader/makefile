CC = gcc
NASM = nasm

CFLAGS = -m32 -Wall -fno-pie
NASMFLAGS = -f elf32

all: loader

startup.o: startup.s
	$(NASM) $(NASMFLAGS) startup.s -o startup.o

loader: loader.c startup.o
	$(CC) $(CFLAGS) loader.c startup.o -o loader

clean:
	rm -f loader startup.o
