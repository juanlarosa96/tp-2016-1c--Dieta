OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "Swap.c"
	gcc -I"/../../LibreriasSO/"  -c  "funciones.c"
	gcc -L"/../../LibreriasSO/" -o "Swap"  Swap.o  funciones.o   -lLibreriasSO -lcommons
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf Swap