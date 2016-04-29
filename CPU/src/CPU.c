#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void){
	t_config* cfg = config_create("example.config");

	//socket cliente nucleo
	struct sockaddr_in direccionServidorNucleo;
	direccionServidorNucleo.sin_family = AF_INET;
	direccionServidorNucleo.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServidorNucleo.sin_port = htons(8080);

	int clienteNucleo = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteNucleo, (void*) &direccionServidorNucleo, sizeof(direccionServidorNucleo)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}

	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		send(clienteNucleo, mensaje, strlen(mensaje), 0);
	}

	//cliente de UMC
	struct sockaddr_in direccionServidorUMC;
	direccionServidorUMC.sin_family = AF_INET;
	direccionServidorUMC.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServidorUMC.sin_port = htons(9000);

	int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}

	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		send(clienteUMC, mensaje, strlen(mensaje), 0);
	}

	return 0;

}
