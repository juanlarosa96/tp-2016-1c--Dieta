#include "funciones.h"
#include "commons/log.h"

AnSISOP_funciones functions = { .AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar = dereferenciar, .AnSISOP_asignar = asignar,
		.AnSISOP_imprimir = imprimir, .AnSISOP_imprimirTexto = imprimirTexto,

};
AnSISOP_kernel kernel_functions = { };

int main(int argc, char **argv) {

	if (argc != 3) {
		printf("Número incorrecto de parámetros\n");
		return -1;
	}

	t_config* config = config_create(argv[1]);

	int PUERTO_NUCLEO = config_get_int_value(config, "PUERTO_NUCLEO");
	char* IP_NUCLEO = config_get_string_value(config, "IP_NUCLEO");

	char ruta[strlen(argv[2])];

	strcpy(ruta, argv[2]); //Recibe ruta de programa ansisop
	FILE* archivo;
	archivo = fopen(ruta, "r");
	if (archivo == NULL) {
		puts("ERROR");
	}

	int largoLinea = 100;
	char *linea = (char *) malloc(sizeof(char) * largoLinea); //Buffer de linea

	char caracter = getc(archivo);
	int codigoValido = 0;	//Flag. Indica si esta dentro del begin-end

	while (caracter != EOF) {	//Lee linea por linea
		int contador = 0;

		while ((caracter != '\n') && (caracter != EOF)) {
			linea[contador] = caracter;
			contador++;
			caracter = getc(archivo);
		}

		linea[contador] = '\0';

		int comentario = 0;
		int i;
		for (i = 0; i < contador; i++) {
			if (linea[i] == '#') {
				comentario = 1;
			}
		}

		if (!comentario) { //Saltea lineas de comentario

			if (!strcmp(linea, "end")) {
				codigoValido = 0;
			}

			//Ejecutar parser con la Linea
			if (codigoValido) {
				printf("%s\n", linea);

				analizadorLinea(strdup(linea), &functions, &kernel_functions);
			}

			if (!strcmp(linea, "begin")) {
				codigoValido = 1;

			}
		}

		caracter = getc(archivo);
	}

	char comando[10];
	printf("\nEscriba comando\n");
	scanf("%s", comando);
	while (strcmp(comando, "Prueba")) {
		printf("Comando no valido \n");
		scanf("%s", comando);
	}

	//Creo log para la consola
	t_log* 	logger;
	logger = log_create("Consola.log", "CONSOLA", 1, log_level_from_string("INFO"));
	char *texto;
	texto= "info";

	struct sockaddr_in direccionNucleo;

	direccionNucleo.sin_family = AF_INET;
	direccionNucleo.sin_addr.s_addr = inet_addr(IP_NUCLEO);
	direccionNucleo.sin_port = htons(PUERTO_NUCLEO);

	int socketNucleo = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(socketNucleo, (void*) &direccionNucleo, sizeof(direccionNucleo))
			!= 0) {
		perror("No se pudo conectar");
		log_error(logger, "No se pudo conectar al nucleo", texto);
		return 1;
	}


	log_info(logger, "Se conectó al núcleo", texto);

	char * mensaje = malloc(100);
	printf("Escriba mensaje: \n");
	scanf("%s", mensaje);
	send(socketNucleo, mensaje, strlen(mensaje), 0);
	log_info(logger, "Envió un mensaje a núcleo", texto);
	log_destroy(logger);



	return EXIT_SUCCESS;
}

t_puntero definirVariable(t_nombre_variable variable) {
	printf("Defino variable\n");
	return variable;
}
t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtengo posición variable\n");
	return variable;
}
t_valor_variable dereferenciar(t_puntero puntero) {
	printf("Dereferenciar\n");
	return puntero;
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignar\n");
}
void imprimir(t_valor_variable valor) {
	printf("Imprimir\n");
}
void imprimirTexto(char* texto) {
	printf("Imprimir texto\n");
}




