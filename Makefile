main: synchronization.o main.o
        gcc --std=c99 --pedantic -Wall -Wmissing-prototypes -Wcpp -g -o main synchronization.o main.o
 
synchronization.o: synchronization.c synchronization.h
        gcc --std=c99 --pedantic -Wall -Wmissing-prototypes -Wcpp -g -o synchronization.o -c synchronization.c
 
main.o: main.c synchronization.h
        gcc --std=c99 --pedantic -Wall -Wmissing-prototypes -Wcpp -g -o main.o -c main.c
 
clean:
        rm -f *.o
 
mrproper: clean
        rm -f main
