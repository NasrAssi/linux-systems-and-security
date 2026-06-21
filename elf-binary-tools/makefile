CC = gcc
CFLAGS = -m32 -g -Wall
LDFLAGS = -m32

all: task1 task4

# Build hexeditplus (task1)
task1: task1.c
	$(CC) $(CFLAGS) -o $@ task1.c

# Build patched digit counter (task4)
task4: task4.c
	$(CC) $(CFLAGS) -fno-pie -fno-stack-protector -o $@ task4.c

clean:
	rm -f task1 task4
