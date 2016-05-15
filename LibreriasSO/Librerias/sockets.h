#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>

enum {

	IDCONSOLA = 1, IDNUCLEO = 2, IDCPU = 3, IDUMC = 4, IDSWAP = 5
};

int crearSocket(int *unSocket);
int escucharEn(int unSocket, int puerto);
int conectarA(int unSocket, char* ip, int puerto);
int iniciarHandshake(int socketDestino, uint8_t idOrigen, uint8_t idEsperado);
int responderHandshake(int socketDestino, uint8_t idOrigen, uint8_t idEsperado);
int aceptarConexion(int socketServidor, struct sockaddr_in * direccionCliente);
void recibirTodo(int socketOrigen,void * buffer, int largo);

