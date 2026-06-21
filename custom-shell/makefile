all: myshell mypipeline looper

LineParser.o: LineParser.c LineParser.h
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c

myshell.o: myshell.c LineParser.h
	gcc -m32 -Wall -g -c -o myshell.o myshell.c

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -g -o myshell myshell.o LineParser.o 

looper.o: looper.c
	gcc -m32 -g -Wall -c -o looper.o looper.c

looper: looper.o
	gcc -m32 -g -Wall -o looper looper.o

mypipeline: mypipeline.c
	gcc -m32 -g -Wall -o mypipeline mypipeline.c 

clean:
	rm -f myshell.o LineParser.o mypipeline myshell looper looper.o
