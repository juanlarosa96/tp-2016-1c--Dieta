#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static const int PUERTO_SERVIDOR = 8080;

int main(void){
	//t_config* cfg = config_create("/home/utnso/workspace/Nucleo/Configuracion/config");

	//char * value = config_get_string_value(cfg, "PUERTO_CPU");
	//puts(value);

	//socket SERVIDOR, falta refactor

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

		//send(socketConsola, "Hola NetCat!", 13, 0);

		char* buffer = malloc(7);

		int bytesRecibidos = recv(socketConsola, buffer, 7, 0);

		printf("Consola dice: %s\n", buffer);


		listen(servidorNucleo, 100); //Esperando que se conecte el CPU

		int socketCpu = accept(servidorNucleo, (void*) &direccionCliente, &len);

		send(socketCpu, buffer, 7, 0);

		free(buffer);




	return EXIT_SUCCESS;

	//servidor para consola y cpu
	//cliente de umc

}
