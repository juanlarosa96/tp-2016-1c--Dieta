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
	NUCLEO = 3,
	CPU = 4,
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
	setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidorUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
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

	char* buffer = malloc(100);

	//int bytesRecibidos = recv(socketCPU, buffer, strlen(buffer), 0);

	//printf("CPU dice: %s\n", buffer);

	while (1) {
		recv(socketCPU, buffer, 100, 0);
		printf("%s\n", buffer);

		/*if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);*/
	}

	free(buffer);

	return EXIT_SUCCESS;

	//servidor de nucleo y cpu
	//cliente de swap

}
