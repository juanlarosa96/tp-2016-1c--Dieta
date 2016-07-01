OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "UMC.c"
	gcc -I"/../../LibreriasSO/"  -c  "funcionesUMC.c"
	gcc -I"/../../LibreriasSO/"  -c  "hexdump.c"
	gcc -L"/../../LibreriasSO/" -o "UMC"  UMC.o  funcionesUMC.o  hexdump.o -lLibreriasSO -lcommons -pthread
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf UMC