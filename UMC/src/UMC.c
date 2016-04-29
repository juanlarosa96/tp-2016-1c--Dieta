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
#include <commons/config.h>

enum handshake {
	NUCLEO = 3, CPU = 4,
};

int main(int argc, char *argv[]) {

	//aca creo un hilo para la consola UMC

	//Recibe el archivo de config por parametro
	if (argc != 2) {
		printf("Número incorrecto de parámetros\n");
		return -1;
	}

	t_config* config = config_create(argv[1]);

	int puerto_servidor = config_get_int_value(config,"PUERTO");
	int puerto_swap = config_get_int_value(config,"PUERTO_SWAP");
	int ip_swap = config_get_int_value(config,"IP_SWAP");


	/*---------SOCKET SERVIDOR------------*/

	struct sockaddr_in direccionServidorUMC;
	direccionServidorUMC.sin_family = AF_INET;
	direccionServidorUMC.sin_addr.s_addr = INADDR_ANY;
	direccionServidorUMC.sin_port = htons(puerto_servidor);

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
	//falta handshake
	int socketCPU = accept(servidorUMC, (void*) &direccionCliente, &len);
	printf("Recibí una conexión de la CPU\n");

	char* bufferServidor = malloc(100);
	recv(socketCPU, bufferServidor, 100, 0);
	printf("%s\n", bufferServidor);

	free(bufferServidor);

	/*---------FIN SOCKET SERVIDOR-------*/

	/*---------SOCKET CLIENTE------------*/

	struct sockaddr_in direccionSwap;
	direccionSwap.sin_family = AF_INET;
	direccionSwap.sin_addr.s_addr = inet_addr(ip_swap);
	direccionSwap.sin_port = htons(puerto_swap);

	int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteUMC, (void*) &direccionSwap, sizeof(direccionSwap)) != 0) {
		perror("No se pudo conectar con el socket de SWAP");
		return 1;
	}

	char bufferCliente[100];
	scanf("%s\n", bufferCliente);
	send(clienteUMC, bufferCliente, 100, 0);

	free(bufferCliente);

	/*---------FIN SOCKET CLIENTE-------*/

	config_destroy(config);

	return EXIT_SUCCESS;

}
