#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void){
	t_config* cfg = config_create("/home/utnso/workspace/Nucleo/Configuracion/config");

	char * value = config_get_string_value(cfg, "PUERTO_CPU");
	puts(value);

	//socket SERVIDOR, falta refactor

	struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(9050);

		int servidorNucleo = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(servidorNucleo, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidorNucleo, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) //bind reserva un puerto
		{
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidorNucleo, 100);


		//------------------------------

		struct sockaddr_in direccionCliente;
		unsigned int len;
		len = sizeof(struct sockaddr_in);
		int cliente = accept(servidorNucleo, (void*) &direccionCliente, &len);

		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "Hola NetCat!", 13, 0);
		send(cliente, ":)\n", 4, 0);

		//------------------------------

		char* buffer = malloc(1000);

		while (1) {
			int bytesRecibidos = recv(cliente, buffer, 1000, 0);
			if (bytesRecibidos <= 0) {
				perror("El chabón se desconectó o bla.");
				return 1;
			}

			buffer[bytesRecibidos] = '\0';
			printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		}

		free(buffer);

		//fin socket SERVIDOR



	return EXIT_SUCCESS;

	//servidor para consola y cpu
	//cliente de umc

}
