#include "funciones.h"
#include <commons/log.h>
#include <Librerias/sockets.h>

int main(int argc, char **argv) {

	t_config* config;
	if (argc != 3) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}

	int PUERTO_NUCLEO = config_get_int_value(config, "PUERTO_NUCLEO");
	char* IP_NUCLEO = config_get_string_value(config, "IP_NUCLEO");

	char ruta[strlen(argv[2])];

	strcpy(ruta, argv[2]); //Recibe ruta de programa ansisop

	char comando[10];
	printf("\nEscriba comando\n");
	scanf("%s", comando);
	while (strcmp(comando, "Prueba")) {
		printf("Comando no valido \n");
		scanf("%s", comando);
	}

	//Creo log para la consola
	t_log* logger;
	logger = log_create("Consola.log", "CONSOLA", 1,
			log_level_from_string("INFO"));
	char *texto;
	texto = "info";

	/*
	 struct sockaddr_in direccionNucleo;

	 direccionNucleo.sin_family = AF_INET;
	 direccionNucleo.sin_addr.s_addr = inet_addr(IP_NUCLEO);
	 direccionNucleo.sin_port = htons(PUERTO_NUCLEO);

	 int socketNucleo = socket(AF_INET, SOCK_STREAM, 0);

	 if (connect(socketNucleo, (void*) &direccionNucleo, sizeof(direccionNucleo)) != 0) {
	 */

	int socketNucleo;
	crearSocket(&socketNucleo);

	if (conectarA(socketNucleo, IP_NUCLEO, PUERTO_NUCLEO) != 0) {
		perror("No se pudo conectar");
		log_error(logger, "No se pudo conectar al nucleo", texto);
		return 1;
	}



	//if (responderHandshake(socketNucleo, IDCONSOLA, IDNUCLEO)) {
	if (responderHandshake(socketNucleo, 1, 2)) {
		log_error(logger, "Error en el handshake", texto);
		return 1;
	}

	log_info(logger, "Se conectó al núcleo", texto);


	send(socketNucleo, ruta, 30, 0);

	char * programaAnsisop;
	int largoPrograma;

	enviarProgramaAnsisop(socketNucleo,programaAnsisop,largoPrograma);
	log_info(logger, "Envió un mensaje a núcleo \n", texto);


	log_destroy(logger);

	return EXIT_SUCCESS;
}
