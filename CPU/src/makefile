OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "CPU.c"
	gcc -I"/../../LibreriasSO/"  -c  "funciones.c"
	gcc -I"/../../LibreriasSO/"  -c  "primitivas.c"
	gcc -L"/../../LibreriasSO/" -o "CPU"  CPU.o  funciones.o primitivas.o   -lLibreriasSO -lparser-ansisop -lcommons -pthread 
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf CPU