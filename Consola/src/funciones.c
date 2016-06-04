/*
 * funciones.c
 *
 *  Created on: 3/6/2016
 *      Author: utnso
 */
#include "funciones.h"

void interpreteComandos(int * socketNucleo) {

	while (1) {
		char comando[100];
		scanf("%s", comando);
		if (!strcmp(comando, "KILL")) {
			int header = finalizacionPrograma;
			send((*socketNucleo), &header, sizeof(int), 0);
			pthread_exit(NULL);
		}
	}

}
