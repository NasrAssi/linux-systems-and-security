all: multi

multi: multi.o
	gcc -m32 -g multi.o -o multi     

multi.o: multi.s
	nasm -f elf32 -g multi.s -o multi.o   

clean:
	rm -f multi.o multi
