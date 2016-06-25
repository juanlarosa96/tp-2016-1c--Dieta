/*
 * funciones.c
 *
 *  Created on: 3/6/2016
 *      Author: utnso
 */
#include "funciones.h"

void interpreteComandos(int * socketNucleo) {
	int conexion = *socketNucleo;
	free(socketNucleo);

	while (1) {
		char * comando = malloc(20);
		printf("Ingrese comando: ");
		scanf("%s", comando);
		if (!strcmp(comando, "KILL")) {
			int header = finalizacionPrograma;
			send(conexion, &header, sizeof(int), 0);
			log_info(logger, "Se envió comando de finalización programa a Núcleo");
			pthread_exit(NULL);
		}
	}

}
