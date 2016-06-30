OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "Nucleo.c"
	gcc -I"/../../LibreriasSO/"  -c  "funciones.c"
	gcc -L"/../../LibreriasSO/" -o "Nucleo"  Nucleo.o  funciones.o   -lLibreriasSO -lparser-ansisop -lcommons -pthread 
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf Nucleo