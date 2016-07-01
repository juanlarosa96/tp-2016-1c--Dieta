OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "Consola.c"
	gcc -I"/../../LibreriasSO/"  -c  "funciones.c"
	gcc -L"/../../LibreriasSO/" -o "Consola"  Consola.o  funciones.o   -lLibreriasSO -lcommons -pthread
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf Consola