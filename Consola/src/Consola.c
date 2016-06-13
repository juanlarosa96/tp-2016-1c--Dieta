#include "funciones.h"
#include "commons/log.h"


int main(int argc, char **argv) {

	t_config* config;
	if (argc != 3) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[2]);
	}

	int PUERTO_NUCLEO = config_get_int_value(config, "PUERTO_NUCLEO");
	char* IP_NUCLEO = config_get_string_value(config, "IP_NUCLEO");

	char ruta[strlen(argv[1])+1];

	strcpy(ruta, argv[1]); //Recibe ruta de programa ansisop

	//Creo log para la consola
	t_log* logger;
	logger = log_create("Consola.log", "CONSOLA", 1,
			log_level_from_string("INFO"));
	char *texto;
	texto = "info";

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
	    int i;

	    for(i = 0; i <largoPrograma;i++){
	    	programaAnsisop[i] = fgetc(archivo);
	    }
	    fclose(archivo);

	    programaAnsisop[largoPrograma] = '\0';

	enviarProgramaAnsisop(socketNucleo,programaAnsisop,largoPrograma+1);
	log_info(logger, "Envió un mensaje a núcleo \n", texto);

	pthread_t nuevoHilo;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&nuevoHilo, &attr, (void *) &interpreteComandos, (void *) &socketNucleo);
	pthread_attr_destroy(&attr);

	while(1){
		int header = recibirHeader(socketNucleo);
		if (header == resultadoEjecucion) {
		int largoMensaje;
		largoMensaje = recibirLargoResultadoDeEjecucionAnsisop(socketNucleo);
		char mensajeDevuelto[largoMensaje];
		recibirResultadoDeEjecucionAnsisop(socketNucleo,mensajeDevuelto,largoMensaje);
		printf(mensajeDevuelto);
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
