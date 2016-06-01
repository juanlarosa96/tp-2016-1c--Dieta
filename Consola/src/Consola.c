#include "funciones.h"
#include "commons/log.h"
#include <sockets.h>
#include <protocolo.h>

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



	if (responderHandshake(socketNucleo, IDCONSOLA, IDNUCLEO)) {
		log_error(logger, "Error en el handshake", texto);
		return 1;
	}

	log_info(logger, "Se conectó al núcleo", texto);

	unsigned long largoPrograma;

	FILE * archivo = fopen(ruta, "r");
	    fseek(archivo, 0, SEEK_END);
	    largoPrograma = (unsigned long)ftell(archivo);
	    fseek(archivo, 0, SEEK_SET);

	    char programaAnsisop[largoPrograma+1];
	    int i = 0;

	    for(i; i <largoPrograma;i++){
	    	programaAnsisop[i] = fgetc(archivo);
	    }
	    fclose(archivo);

	    programaAnsisop[largoPrograma+1] = '\0';

	//send(socketNucleo, ruta, 30, 0);


	enviarProgramaAnsisop(socketNucleo,programaAnsisop,largoPrograma);
	log_info(logger, "Envió un mensaje a núcleo \n", texto);

	while(1){
		int header = recibirHeader(socketNucleo);
		if (header == resultadoEjecucion) {
		int largoMensaje;
		largoMensaje = recibirLargoResultadoDeEjecucionAnsisop(socketNucleo);
		char mensajeDevuelto[largoMensaje];
		recibirResultadoDeEjecucionAnsisop(socketNucleo,mensajeDevuelto,largoMensaje);
		}

		if(header == finalizacionPrograma){
			close(socketNucleo);
			log_info(logger, "Programa finalizado", texto);
			return EXIT_SUCCESS;

		}

	}


	log_destroy(logger);

	return EXIT_SUCCESS;
}
