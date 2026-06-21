CC = gcc
CFLAGS = -m32 -g -Wall

all: hexeditplus digit_counter

# Interactive hex editor / memory tool
hexeditplus: hexeditplus.c
	$(CC) $(CFLAGS) -o $@ hexeditplus.c

# Small program used as a gdb binary-patching target
digit_counter: digit_counter.c
	$(CC) $(CFLAGS) -fno-pie -fno-stack-protector -o $@ digit_counter.c

clean:
	rm -f hexeditplus digit_counter
