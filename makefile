CFLAG = cc -c -Wall
all: run clean
run: dep.o grep.o 
	cc grep.o dep.o -o run
dep.o  : dep.c head.h
	$(CFLAG) head.h dep.c 
grep.o : grep.c
	$(CFLAG) grep.c
clean:
	rm *.o
