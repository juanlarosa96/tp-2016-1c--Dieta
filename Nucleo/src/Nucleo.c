#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "commons/log.h"

int main(int argc, char **argv){

	if (argc != 2) {
			printf("Número incorrecto de parámetros\n");
			return -1;
		}

	t_config* config = config_create(argv[1]);

	int PUERTO_SERVIDOR = config_get_int_value(config, "PUERTO_SERVIDOR");
	int PUERTO_UMC = config_get_int_value(config, "PUERTO_UMC");
	char* IP_UMC = config_get_string_value(config, "IP_UMC");
	//puts(value);

	//Creo log para el Núcleo
	t_log* 	logger;
	logger = log_create("Núcelo.log", "NUCELO", 1, log_level_from_string("INFO"));
	char *texto;
	texto= "info";

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
			log_error(logger, "Se produjo un error creando el socket servidor", texto);
			return 1;
		}

		log_info(logger, "Se estableciió correctamente el socket servidor", texto);
		printf("Escuchando\n");
		listen(servidorNucleo, 100);


		//------------------------------

		struct sockaddr_in direccionCliente;
		unsigned int len;
		len = sizeof(struct sockaddr_in);
		int socketConsola = accept(servidorNucleo, (void*) &direccionCliente, &len);

		log_info(logger, "Se conectó con la consola", texto);

		printf("Recibí una conexión de la consola \n");

		int bytesRecibidos;
		char* ruta = malloc(30);
		bytesRecibidos = recv(socketConsola, ruta, 30, 0);
		ruta[bytesRecibidos] = '\0';

		char* buffer = malloc(10);
		bytesRecibidos = recv(socketConsola, buffer, 10, 0);
		buffer[bytesRecibidos] = '\0';

		printf("Consola dice: %s\n", buffer);
		log_info(logger, "Se recibió un mensaje", texto);


		/*struct sockaddr_in direccionUmc;

			direccionUmc.sin_family = AF_INET;
			direccionUmc.sin_addr.s_addr = inet_addr(IP_UMC);
			direccionUmc.sin_port = htons(PUERTO_UMC);

			int socketUmc = socket(AF_INET, SOCK_STREAM, 0);

			if (connect(socketUmc, (void*) &direccionUmc , sizeof(direccionUmc))
					!= 0) {
				perror("No se pudo conectar");
				return 1;
			}

			send(socketUmc, buffer, 100, 0);*/



		listen(servidorNucleo, 100); //Esperando que se conecte el CPU

		int socketCpu = accept(servidorNucleo, (void*) &direccionCliente, &len);

		printf("Se conecto el CPU");


		send(socketCpu,ruta,30,0);

		log_info(logger, "Se conectó al CPU", texto);

		send(socketCpu, buffer, 10, 0);

		log_info(logger, "Se envió un buffer al CPU", texto);

		free(buffer);

		log_destroy(logger);




	return EXIT_SUCCESS;

	//servidor para consola y cpu
	//cliente de umc

}
