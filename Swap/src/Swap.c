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
#include <commons/config.h>

int main(int argc, char *argv[]) {

	if (argc != 2) {
			printf("Número incorrecto de parámetros\n");
			return -1;
		}

	t_config *config = config_create(argv[1]);

	int puertoServidor =   config_get_int_value(config,"PUERTO_ESCUCHA");
	int cantidadDePaginas =   config_get_int_value(config,"CANTIDAD_PAGINAS");
	int sizePagina =   config_get_int_value(config,"TAMANIO_PAGINA");
	int retardoCompactacion =   config_get_int_value(config,"RETARDO_COMPACTACION");

	char*nombre = config_get_string_value(config,"NOMBRE_SWAP");


//---------------------------------------------------------------------------
	struct sockaddr_in direccionServidorSwap;
	direccionServidorSwap.sin_family = AF_INET;
	direccionServidorSwap.sin_addr.s_addr = INADDR_ANY;
	direccionServidorSwap.sin_port = htons(puertoServidor);

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
	printf("Recibí una conexión\n");

	//------------------------------

	char* buffer = malloc(10);

	int bytesRecibidos = recv(cliente, buffer, 10, 0);
	buffer[bytesRecibidos] = '\0';

	printf("UMC dice: %s\n", buffer);

	//printf("Me llegaron %d bytes con %s", bytesRecibidos, buffer;

	free(buffer);
//---------------------------------------------------------------------------
	config_destroy(config);

	return 0;
}
