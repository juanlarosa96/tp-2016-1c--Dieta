OBJS := ./pila.o ./protocolo.o ./sockets.o

all:
	make clean
	make obj
	gcc -shared -o "libLibreriasSO.so" $(OBJS)
	rm -rf socket.o

obj:
	gcc ./sockets.c -c
	gcc ./pila.c -c
	gcc ./protocolo.c -c

clean:
	rm -rf libLibreriasSO.so
	rm -rf sockets.o
	rm -rf pila.o
	rm -rf protocolo.o

install:
	make all
	cp -u ./libLibreriasSO.so ~/../../usr/lib/libLibreriasSO.so
	cp --parents -u ./*.h /usr/include