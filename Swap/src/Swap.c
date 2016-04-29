/*
 ============================================================================
 Name        : Swap.c
 Author      : Dieta
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<commons/config.h>

static const int PUERTO_SERVIDOR_SWAP = 9001;

int main(void) {
	struct sockaddr_in direccionServidorSwap;
	direccionServidorSwap.sin_family = AF_INET;
	direccionServidorSwap.sin_addr.s_addr = INADDR_ANY;
	direccionServidorSwap.sin_port = htons(PUERTO_SERVIDOR_SWAP);

	int servidorSwap = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidorSwap, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidorSwap, (void*) &direccionServidorSwap, sizeof(direccionServidorSwap)) != 0) {
		perror("Falló el bind");
		return 1;
	}

	printf("Estoy escuchando\n");
	listen(servidorSwap, 100);

	//------------------------------

	struct sockaddr_in direccionCliente;
	unsigned int len;
	len = sizeof(struct sockaddr_in);

	int cliente = accept(servidorSwap, (void*) &direccionCliente, &len);
	printf("Recibí una conexión en %d!!\n", cliente);

	//------------------------------

	char* bufferServidor = malloc(100);

	int bytesRecibidos = recv(cliente, bufferServidor, 100, 0);
	if (bytesRecibidos < 0) {
		perror("Se Desconecto");
		return 1;
	}

	bufferServidor[bytesRecibidos] = '\0';
	printf("Me llegaron %d bytes con %s", bytesRecibidos, bufferServidor);

	free(bufferServidor);

	return 0;
}
