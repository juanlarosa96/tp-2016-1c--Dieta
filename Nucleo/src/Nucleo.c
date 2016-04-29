#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv){
	argc = 2;
	if (argc != 2) {
			printf("Número incorrecto de parámetros\n");
			return -1;
		}

	argv[1] = "/home/utnso/Escritorio/TP/tp-2016-1c--Dieta/Nucleo/Configuracion/config";
	t_config* config = config_create(argv[1]);

	int PUERTO_SERVIDOR = config_get_int_value(config, "PUERTO_SERVIDOR");
	int PUERTO_UMC = config_get_int_value(config, "PUERTO_UMC");
	char* IP_UMC = config_get_string_value(config, "IP_UMC");
	//puts(value);


	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(PUERTO_SERVIDOR);

		int servidorNucleo = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(servidorNucleo, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidorNucleo, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) //bind reserva un puerto
		{
			perror("Falló el bind");
			return 1;
		}

		printf("Escuchando\n");
		listen(servidorNucleo, 100);


		//------------------------------

		struct sockaddr_in direccionCliente;
		unsigned int len;
		len = sizeof(struct sockaddr_in);
		int socketConsola = accept(servidorNucleo, (void*) &direccionCliente, &len);

		printf("Recibí una conexión de la consola \n");

		char* buffer = malloc(100);
		int bytesRecibidos = recv(socketConsola, buffer, 100, 0);
		buffer[bytesRecibidos] = '\0';

		printf("Consola dice: %s\n", buffer);


		struct sockaddr_in direccionUmc;

			direccionUmc.sin_family = AF_INET;
			direccionUmc.sin_addr.s_addr = inet_addr(IP_UMC);
			direccionUmc.sin_port = htons(PUERTO_UMC);

			int socketUmc = socket(AF_INET, SOCK_STREAM, 0);

			if (connect(socketUmc, (void*) &direccionUmc , sizeof(direccionUmc))
					!= 0) {
				perror("No se pudo conectar");
				return 1;
			}

			send(socketUmc, buffer, 100, 0);



		listen(servidorNucleo, 100); //Esperando que se conecte el CPU

		int socketCpu = accept(servidorNucleo, (void*) &direccionCliente, &len);

		printf("Se conecto el CPU");

		send(socketCpu, buffer, 100, 0);

		free(buffer);




	return EXIT_SUCCESS;

	//servidor para consola y cpu
	//cliente de umc

}
