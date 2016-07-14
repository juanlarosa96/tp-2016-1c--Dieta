/*
 * funciones.c
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */
#include "funciones.h"

int iniciarProgramaAnsisop(int cliente, char*archivo) {

	int cantPaginasTotal;
	recibirTodo(cliente, &cantPaginasTotal, sizeof(int));

	int frameInicial = chequearMemoriaDisponible(cantPaginasTotal, archivo);
	if (frameInicial == -1) {
		avisarUMCFallo(cliente);
		log_info(logger, "No hay espacio para inicializar un nuevo programa.");
		return 0;
	} else {
		avisarUMCExito(cliente);
	}

	t_proceso * proceso = malloc(sizeof(t_proceso));

	uint32_t pid;
	recibirTodo(cliente, &pid, sizeof(uint32_t));

	int cantPaginasCodigo;
	recibirTodo(cliente, &cantPaginasCodigo, sizeof(int));

	proceso->pID = pid;
	proceso->frameInicial = frameInicial;
	proceso->cantPaginas = cantPaginasTotal;

	list_add(listaProcesos, proceso);
	int i;
	char *pagina = malloc(sizePagina);

	for (i = 0; i < cantPaginasTotal; i++) {
		if (i < cantPaginasCodigo) {

			recibirTodo(cliente, pagina, sizePagina);
			memcpy(&(archivo[frameInicial * sizePagina]), pagina, sizePagina);
			usleep(retardoAcceso * 1000);

		}
		bitMap[frameInicial] = 1;
		frameInicial++;
	}
	free(pagina);
	log_info(logger, "Se inicializó un nuevo programa PID: %d", pid);
	return 1;
}

void guardarPaginas(int cliente, char*archivo) {

	int nroPagina;
	recibirTodo(cliente, &nroPagina, sizeof(int));

	uint32_t pID;
	recibirTodo(cliente, &pID, sizeof(uint32_t));

	char *pagina = malloc(sizePagina);
	recibirTodo(cliente, pagina, sizePagina);

	int i;
	t_proceso *procesoAux;
	for (i = 0; i < list_size(listaProcesos); i++) {
		procesoAux = list_get(listaProcesos, i);

		if (procesoAux->pID == pID) {
			usleep(retardoAcceso * 1000);
			memcpy(&(archivo[(procesoAux->frameInicial + nroPagina) * sizePagina]), pagina, sizePagina);
			break;
		}

	} //aca no le falta el free de la pagina?

	log_info(logger, "Se almacenó página nro %d del proceso PID %d.", nroPagina, pID);
}

void enviarPaginas(int cliente, char*archivo) {
	uint32_t pID;
	recibirTodo(cliente, &pID, sizeof(uint32_t));

	int nroPagina;
	recibirTodo(cliente, &nroPagina, sizeof(int));

	int i;
	t_proceso *procesoAux;
	for (i = 0; i < list_size(listaProcesos); i++) {
		procesoAux = list_get(listaProcesos, i);

		if (procesoAux->pID == pID) {
			usleep(retardoAcceso * 1000);
			send(cliente, &(archivo[(procesoAux->frameInicial + nroPagina) * sizePagina]), sizePagina, 0);
			break;
		}

	}
	log_info(logger, "Se envió página nro %d, del proceso con PID %d, a UMC", nroPagina, pID);

}

void finalizarProgramaAnsisop(int cliente) {

	uint32_t pID;
	recibirTodo(cliente, &pID, sizeof(uint32_t));

	int i;
	t_proceso *procesoAux;
	for (i = 0; i < list_size(listaProcesos); i++) {
		procesoAux = list_get(listaProcesos, i);
		if (procesoAux->pID == pID) {
			int a;
			for (a = 0; a < procesoAux->cantPaginas; a++) {
				bitMap[a + procesoAux->frameInicial] = 0;
			}
			free(list_remove(listaProcesos, i));
		}
	}

	log_info(logger, "Fin de programa PID: %d", pID);
}

int chequearMemoriaDisponible(int cantPaginas, char*archivo) {
	int cantidadContinua, i, cantidadTotal;
	cantidadContinua = i = cantidadTotal = 0;
	int hayContinuas = 0, hayTotales = 0;

	while (i < cantidadDeFrames && !hayContinuas) {
		if (bitMap[i] == 0) {
			cantidadTotal++;
			cantidadContinua++;
		} else {
			cantidadContinua = 0;
		}
		if (cantidadTotal == cantPaginas) {
			hayTotales = 1;
		}
		if (cantidadContinua == cantPaginas) {
			hayContinuas = 1;
		}
		i++;
	}

	if (hayTotales) {
		if (hayContinuas) {
			return i - cantPaginas;
		} else {
			return compactar(archivo);
		}
	} else {
		return -1;
	}
}

void avisarUMCFallo(int cliente) {
	int header = inicioProgramaError;
	send(cliente, &header, sizeof(int), 0);
}

void avisarUMCExito(int cliente) {
	int header = inicioProgramaExito;
	send(cliente, &header, sizeof(int), 0);
}

int compactar(char*archivo) {
	int i, j;
	int ultimoFrameLibre = 0;
	log_info(logger, "Comienzo de compactación.");
	for (i = 0; i < cantidadDeFrames - 1; i++) {
		if (bitMap[i] == 0 && bitMap[i + 1] == 1) {
			memcpy(&(archivo[i * sizePagina]), &(archivo[(i + 1) * sizePagina]), sizePagina);
			bitMap[i] = 1;
			bitMap[i + 1] = 0;
			for (j = 0; j < list_size(listaProcesos); j++) {

				t_proceso * proceso = list_get(listaProcesos, j);
				if (proceso->frameInicial == i + 1) {
					proceso->frameInicial--;
					break;

				}
				i = ultimoFrameLibre - 1;

			}

		} else if (bitMap[i] == 1) {
			ultimoFrameLibre++;
		}
	}
	usleep(retardoCompactacion * 1000);
	log_info(logger, "Fin compactación.");
	return ultimoFrameLibre;
}

