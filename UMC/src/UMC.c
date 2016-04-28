/*
 ============================================================================
 Name        : UMC.c
 Author      : Dieta
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

enum handshake {
	NUCLEO = 3, CPU = 4,
};

static const int PUERTO_SERVIDOR_UMC = 9000;
static const int PUERTO_SWAP = 9001;

int main(void) {

	//aca creo un hilo para la consola UMC

	/*---------SOCKET SERVIDOR------------*/

	struct sockaddr_in direccionServidorUMC;
	direccionServidorUMC.sin_family = AF_INET;
	direccionServidorUMC.sin_addr.s_addr = INADDR_ANY;
	direccionServidorUMC.sin_port = htons(PUERTO_SERVIDOR_UMC);

	int servidorUMC = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado,
			sizeof(activado));

	if (bind(servidorUMC, (void*) &direccionServidorUMC,
			sizeof(direccionServidorUMC)) != 0) {
		perror("Falló el bind");
		return 1;
	}

	printf("Estoy escuchando\n");
	listen(servidorUMC, 100);

	//clientes de UMC: núcleo, CPU.

	struct sockaddr_in direccionCliente;
	unsigned int len;
	len = sizeof(struct sockaddr_in);

	//Acepta la conexion con CPU
	int socketCPU = accept(servidorUMC, (void*) &direccionCliente, &len);
	printf("Recibí una conexión de la CPU\n");

	char* bufferServidor = malloc(100);
	recv(socketCPU, bufferServidor, 100, 0);
	printf("%s\n", bufferServidor);

	free(bufferServidor);

	/*---------SOCKET CLIENTE------------*/

	struct sockaddr_in direccionSwap;
	direccionSwap.sin_family = AF_INET;
	direccionSwap.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionSwap.sin_port = htons(PUERTO_SWAP);

	int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteUMC, (void*) &direccionSwap, sizeof(direccionSwap)) != 0) {
		perror("No se pudo conectar con el socket de SWAP");
		return 1;
	}

	char bufferCliente[100];
	scanf("%s\n", bufferCliente);
	send(clienteUMC, bufferCliente, 100, 0);


	return EXIT_SUCCESS;

	//servidor de nucleo y cpu
	//cliente de swap

}
