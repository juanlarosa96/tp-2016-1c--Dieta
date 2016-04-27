#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static const int PUERTO_NUCLEO = 8080;

int main(void) {

	struct sockaddr_in direccionServidor;

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServidor.sin_port = htons(PUERTO_NUCLEO);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);


	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}
		char mensaje[1000];
		scanf("%s", mensaje);
		send(cliente, mensaje, strlen(mensaje), 0);


		return EXIT_SUCCESS;
	}
