main: synchronization.o main.o
        gcc -o main synchronization.o main.o
 
synchronization.o: synchronization.c synchronization.h
        gcc -Wall -o synchronization.o -c synchronization.c
 
main.o: main.c synchronization.h
        gcc -Wall -o main.o -c main.c
 
clean:
        rm -f *.o
 
mrproper: clean
        rm -f main
