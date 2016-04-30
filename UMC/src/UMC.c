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

	//Crear hilo para la consola UMC

	//Recibe el archivo de config por parametro
	if (argc != 2) {
		printf("Número incorrecto de parámetros\n");
		return -1;
	}

	t_config* config = config_create(argv[1]);

	int puerto_servidor = config_get_int_value(config,"PUERTO");
	int puerto_swap = config_get_int_value(config,"PUERTO_SWAP");
	char* ip_swap = config_get_string_value(config,"IP_SWAP");

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

	int socketCliente = accept(servidorUMC, (void*) &direccionCliente, &len);
	printf("Recibí una conexión\n");

	char* bufferServidor = malloc(100);
	int bytesRecibidos = recv(socketCliente, bufferServidor, 100, 0); //Recibe "HEADER: Hola UMC"
	bufferServidor[bytesRecibidos] = '\0';

	/* FALTA IMPLEMENTAR HANDSHAKE
	char * quienSos = string_substring_until(bufferServidor, 1); //Supongo que el header tiene 1 byte
	int quienMeHabla = atoi(quienSos); //capaz esto me conviene encapsularlo en una funcion

	switch(quienMeHabla){
		case NUCLEO: send(socketCliente, "Hola Núcleo!", 13, 0);
		break;
		case CPU: send(socketCliente,"Hola CPU!",10, 0);
		break;
		default:
			send(socketCliente,"No te conozco",100,0);
			//aca corta la conexion ?
	}
	*/

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
