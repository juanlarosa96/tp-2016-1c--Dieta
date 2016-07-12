/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

#define OVERFLOW -2

void destruirProceso(t_nodo_lista_procesos * nodo) {
	free(nodo);
}

void destruirFrame(t_nodo_lista_frames * nodo) {
	free(nodo);
}

void destruirPagina(t_nodo_lista_paginas * nodo) {
	free(nodo);
}

void liberarPaginas(int indiceListaProceso) {
	t_nodo_lista_procesos * procesoAux;
	procesoAux = list_get(listaProcesos, indiceListaProceso);
	list_clean_and_destroy_elements(procesoAux->lista_paginas,
			(void*) destruirPagina);

}

int encontrarPosicionEnListaProcesos(uint32_t pid) {
	t_nodo_lista_procesos * aux;

	int i = 0;
	int encontrado = 0;

	usleep(retardo * 1000);

	pthread_mutex_lock(&mutexProcesos);
	while ((i < list_size(listaProcesos)) && encontrado == 0) {
		aux = list_get(listaProcesos, i);

		if (aux->pid == pid) {
			encontrado = 1;
		} else {
			i++;
		}
	}
	pthread_mutex_unlock(&mutexProcesos);

	if (encontrado == 0) {
		return -1; //no se encontró el pid
	}

	return i;
}

void liberarFrames(uint32_t pid) {
	t_nodo_lista_frames* nodoAux;
	int i = 0;
	pthread_mutex_lock(&mutexFrames);
	while (i < list_size(listaFrames)) {
		nodoAux = list_get(listaFrames, i);
		if (nodoAux->pid == pid) {
			nodoAux->pid = 0;
			//list_replace(listaFrames, i, &nodoAux);
		}
		i++;
	}
	pthread_mutex_unlock(&mutexFrames);
}

int cantidadFramesDisponibles() {
	int i = 0;
	int contador = 0;
	t_nodo_lista_frames* nodoAux;
	//pthread_mutex_lock(&mutexFrames);
	while (i < list_size(listaFrames)) {
		nodoAux = list_get(listaFrames, i);
		if (nodoAux->pid == 0) {
			contador++;
		}
		i++;
	}
	//pthread_mutex_unlock(&mutexFrames);
	return contador;
}

int buscarEnTLB(uint32_t pid, int nroPagina) {
	int frame = -1;

	t_entrada_tlb* nodoAux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB) && acierto == 0) {
		nodoAux = list_get(TLB, i);
		if ((nodoAux->pid == pid)
				&& (nodoAux->nroPagina == (uint32_t) nroPagina)) {
			frame = (int) nodoAux->nroFrame;
			acierto = 1;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexTLB);

	return frame;
}

int buscarEntradaMenosUsadaRecientemente() {

	int ultAccesoAuxiliar;
	int indice = 0;
	int i = 0;
	t_entrada_tlb* entradaAux;

	pthread_mutex_lock(&mutexContadorMemoria);
	ultAccesoAuxiliar = (int) accesoMemoria;
	pthread_mutex_unlock(&mutexContadorMemoria);

	//pthread_mutex_lock(&mutexTLB); OJO, VER SI PONER ANTES DE LRU, NO SE SI ACA ESA BIEN
	while (i < list_size(TLB)) {
		entradaAux = list_get(TLB, i);
		if ((entradaAux->ultAcceso) < ultAccesoAuxiliar) {
			ultAccesoAuxiliar = entradaAux->ultAcceso;
			indice = i;
		}
		i++;
	}
	//pthread_mutex_unlock(&mutexTLB);

	return indice;

}
void entradaTLBdestroy(t_entrada_tlb* self) {
	free(self);
}

void lru(int paginaNueva, uint32_t pid, uint32_t frame, int socketCPU) { //antes de llamar a lru mutex TLB

	int indiceVictima = buscarEntradaMenosUsadaRecientemente();
	uint32_t paginaAnterior, pidAnterior;
	t_entrada_tlb* entradaAuxiliar;

	entradaAuxiliar = list_get(TLB, indiceVictima);
	paginaAnterior = entradaAuxiliar->nroPagina;
	entradaAuxiliar->nroPagina = paginaNueva;
	pidAnterior = entradaAuxiliar->pid;
	entradaAuxiliar->pid = pid;
	entradaAuxiliar->nroFrame = frame;
	pthread_mutex_lock(&mutexContadorMemoria);
	entradaAuxiliar->ultAcceso = accesoMemoria; //OJO VARIABLE GLOBAL
	pthread_mutex_unlock(&mutexContadorMemoria);
	/*list_replace_and_destroy_element(TLB, indiceVictima, entradaAuxiliar,
	 (void *) entradaTLBdestroy);*/
	log_info(logger,
			"Thread CPU %d - Se ejecutó LRU para TLB. Se reemplazó página %d del proceso PID %d por página %d del proceso PID %d.",
			socketCPU, paginaAnterior, pidAnterior, paginaNueva, pid);
}

int buscarEnListaProcesos(uint32_t pid, int nroPagina) {

	uint32_t frame;

	t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_paginas* nodoPagAux;
	int i = 0;
	int j = 0;
	int pidEncontrado = 0, paginaEncontrada = 0;

	usleep(retardo * 1000);

	pthread_mutex_lock(&mutexProcesos);

	while (i < list_size(listaProcesos) && pidEncontrado == 0) {
		nodoAux = list_get(listaProcesos, i);
		if (nodoAux->pid == pid) {
			pidEncontrado = 1;
			while (j < list_size(nodoAux->lista_paginas)
					&& paginaEncontrada == 0) {
				nodoPagAux = list_get(nodoAux->lista_paginas, j);

				if (nodoPagAux->nro_pagina == nroPagina) {
					paginaEncontrada = 1;
					if (nodoPagAux->status == 'M') { //estaEnMemoria
						frame = nodoPagAux->nroFrame;  //aplicarLRU para TLB
					} else {
						pthread_mutex_unlock(&mutexProcesos);
						return -1;
						//buscarEnSwap
					}
				}
				j++;
			} //fin While de lista de paginas
		} //fin if nodoAux == pid
		i++;
	} //fin while lista procesos
	pthread_mutex_unlock(&mutexProcesos);

	if (paginaEncontrada == 0) {
		return OVERFLOW;
	}

	return (int) frame;
}

void limpiarEntradasTLBporPID(uint32_t pid) {
	t_entrada_tlb* nodoAux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB) && acierto == 0) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid) {
			nodoAux->pid = 0;
			acierto = 1;
			//list_replace_and_destroy_element(TLB, i, nodoAux,
			//(void*) entradaTLBdestroy);
		}
		i++;
	}
	pthread_mutex_unlock(&mutexTLB);

}

void flushTLB() {
	t_entrada_tlb* aux;
	int i = 0;
	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB)) {
		aux = list_get(TLB, i);
		aux->pid = 0;

		i++;
	}

	pthread_mutex_unlock(&mutexTLB);
}

void flushMemory() {
	int i;
	t_nodo_lista_frames * nodo;
	pthread_mutex_lock(&mutexFrames);
	for (i = 0; i < list_size(listaFrames); i++) {
		nodo = list_get(listaFrames, i);
		nodo->bitModificado = 1;
		/*list_replace_and_destroy_element(listaFrames, i, nodo,
		 (void*) destruirFrame);*/
	}
	pthread_mutex_unlock(&mutexFrames);
}

void actualizarBitReferencia(uint32_t frame) {
	int i = frame; //ojoo con esto
	int acierto = 0;
	t_nodo_lista_frames * nodoAuxiliar;
	pthread_mutex_lock(&mutexFrames);
	while (i < list_size(listaFrames) && acierto == 0) {
		nodoAuxiliar = list_get(listaFrames, i);
		if (nodoAuxiliar->nroFrame == frame) {
			nodoAuxiliar->bitReferencia = 1;
			/*list_replace_and_destroy_element(listaFrames, i, nodoAuxiliar,
			 (void*) destruirFrame);*/
			acierto = 1;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexFrames);

}

void actualizarBitModificado(uint32_t frame) {
	int i = frame;
	int acierto = 0;
	t_nodo_lista_frames * nodoAuxiliar;
	pthread_mutex_lock(&mutexFrames);
	while (i < list_size(listaFrames) && acierto == 0) {
		nodoAuxiliar = list_get(listaFrames, i);
		if (nodoAuxiliar->nroFrame == frame) {
			nodoAuxiliar->bitModificado = 1;
			/*list_replace_and_destroy_element(listaFrames, i, nodoAuxiliar,
			 (void*) destruirFrame); */			//no estoy segura del destoy
			acierto = 1;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexFrames);

}

void * lecturaMemoria(uint32_t frame, uint32_t offset, uint32_t tamanio) {
	void * bytes = malloc(tamanio);
	int posicion = (int) frame * size_frames + offset;

	usleep(retardo * 1000);
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	void * posicionAux = memoriaPrincipal + posicion;
	memcpy(bytes, posicionAux, tamanio);
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	pthread_mutex_lock(&mutexContadorMemoria);
	accesoMemoria++;
	pthread_mutex_unlock(&mutexContadorMemoria);
	actualizarBitReferencia(frame);
	return bytes;
}

void escrituraMemoria(uint32_t frame, uint32_t offset, uint32_t tamanio,
		void * buffer) {
	int posicion = (int) frame * size_frames + offset;
	usleep(retardo * 1000);
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	void * posicionAux = memoriaPrincipal + posicion;
	memcpy(posicionAux, buffer, tamanio);
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	pthread_mutex_lock(&mutexContadorMemoria);
	accesoMemoria++;
	pthread_mutex_unlock(&mutexContadorMemoria);
	actualizarBitReferencia(frame);
	free(buffer);
}

int moverPuntero(uint32_t pid, int puntero) { //busco el proximo frame que tenga el pid
	t_nodo_lista_frames * nodoFrame;
	int punteroADevolver;
	int acierto = 0;

	puntero++;
	if (puntero == list_size(listaFrames)) {
		puntero = 0;
	}

	while (puntero < list_size(listaFrames) && acierto == 0) {
		nodoFrame = list_get(listaFrames, puntero);
		if (nodoFrame->pid == pid) {
			punteroADevolver = puntero;
			acierto = 1;
		}

		puntero++;
		if (puntero == list_size(listaFrames)) {
			puntero = 0;
		}

	}

	return punteroADevolver;
}

int buscarVictimaClock(t_nodo_lista_procesos* nodoAux, uint32_t pid) {
	//int indice;
	int puntero;
	int nuevoPuntero;
	int victima;
	int acierto = 0;
	//t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_frames* nodoFrame;
	//indice = encontrarPosicionEnListaProcesos(pid);

	//pthread_mutex_lock(&mutexProcesos);
	//nodoAux = list_get(listaProcesos, indice);
	puntero = nodoAux->punteroClock;
	//pthread_mutex_unlock(&mutexProcesos);

	//pthread_mutex_lock(&mutexFrames);
	while (puntero < list_size(listaFrames) && acierto == 0) {
		nodoFrame = list_get(listaFrames, puntero);
		if (nodoFrame->pid == pid) {
			if (nodoFrame->bitReferencia == 0) {
				victima = puntero; //encontre la victima
				nuevoPuntero = moverPuntero(pid, puntero);
				nodoAux->punteroClock = nuevoPuntero; //corro puntero
				acierto = 1;
			} else {
				nodoFrame->bitReferencia = 0;
			}
		}
		puntero++;

		if (puntero == list_size(listaFrames)) {
			puntero = 0;
		}

	}
	//pthread_mutex_unlock(&mutexFrames);

	return victima;

}

int buscarVictimaClockModificado(t_nodo_lista_procesos * nodoAux, uint32_t pid) {
	//int indice;
	int puntero, punteroInicial;
	int nuevoPuntero;
	int victima;
	int acierto = 0;
	int cantVueltas = 0;
	//t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_frames* nodoFrame;
	//indice = encontrarPosicionEnListaProcesos(pid);

	//pthread_mutex_lock(&mutexProcesos);
	//nodoAux = list_get(listaProcesos, indice);
	puntero = nodoAux->punteroClock;
	//pthread_mutex_unlock(&mutexProcesos);
	punteroInicial = puntero;

	//pthread_mutex_lock(&mutexFrames);
	while (puntero < list_size(listaFrames) && acierto == 0) {
		nodoFrame = list_get(listaFrames, puntero);
		if (nodoFrame->pid == pid && cantVueltas == 0) {
			if (nodoFrame->bitReferencia == 0
					&& nodoFrame->bitModificado == 0) {
				victima = puntero; //encontre la victima
				nuevoPuntero = moverPuntero(pid, puntero);
				nodoAux->punteroClock = nuevoPuntero; //corro puntero
				acierto = 1;
			}
		} else if (nodoFrame->pid == pid && cantVueltas == 1) {
			if (nodoFrame->bitReferencia == 0
					&& nodoFrame->bitModificado == 1) {
				victima = puntero; //encontre la victima
				nuevoPuntero = moverPuntero(pid, puntero);
				nodoAux->punteroClock = nuevoPuntero; //corro puntero
				acierto = 1;
			} else {
				nodoFrame->bitReferencia = 0;
			}
		}
		puntero++;

		if (puntero == list_size(listaFrames)) {
			puntero = 0;
		}

		if ((cantVueltas == 0) && (puntero == punteroInicial)) {
			cantVueltas++;
		} else if ((cantVueltas == 1) && (puntero == punteroInicial)) {
			cantVueltas = 0;
		}
	}

	//pthread_mutex_unlock(&mutexFrames);

	return victima;

}

void actualizarNodoPaginaNuevaCargadaEnM(t_nodo_lista_procesos * nodoProceso,
		uint32_t idFrame, uint32_t paginaNueva) {

	t_nodo_lista_paginas * nodoPagina;
	int i = 0;
	int acierto = 0;

	//pthread_mutex_lock(&mutexProcesos);
	//nodoProceso = list_get(listaProcesos, indiceProceso);

	while (i < list_size(nodoProceso->lista_paginas) && acierto == 0) {
		nodoPagina = list_get(nodoProceso->lista_paginas, i);
		if (nodoPagina->nro_pagina == paginaNueva) {
			nodoPagina->nroFrame = idFrame;
			nodoPagina->status = 'M';
			acierto = 1;
		}

		i++;
	}
	//pthread_mutex_unlock(&mutexProcesos);

}

void enviarPaginaASwap(int nroPagina, uint32_t pid, void * pagina) {
	int header = guardarPaginasEnSwap;
	void * data = malloc(sizeof(int) * 2 + sizeof(uint32_t) + size_frames);
	int offset = 0, fallo = 0;
	memcpy(data, &header, sizeof(int));
	offset += sizeof(int);
	memcpy(data + offset, &nroPagina, sizeof(int));
	offset += sizeof(int);
	memcpy(data + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(data + offset, pagina, size_frames);
	offset += size_frames;
	pthread_mutex_lock(&mutexSwap);
	fallo = send(socketSwap, data, offset, MSG_NOSIGNAL);
	pthread_mutex_unlock(&mutexSwap);

	if (fallo == -1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC.");
		abort();
	}

	free(pagina);
	free(data);
}

void actualizarPaginaAReemplazar(t_nodo_lista_procesos * nodoProceso,
		int idFrame, int bitModificado, int socketCPU) {
	//t_nodo_lista_procesos * nodoProceso;
	t_nodo_lista_paginas * nodoPagina;
	int i = 0;

	//pthread_mutex_lock(&mutexProcesos);
	//nodoProceso = list_get(listaProcesos, indiceProceso);

	while (i < list_size(nodoProceso->lista_paginas)) {
		nodoPagina = list_get(nodoProceso->lista_paginas, i);
		if (nodoPagina->nroFrame == (uint32_t) idFrame
				&& nodoPagina->status == 'M') {
			nodoPagina->status = 'S';
			break;
		}

		i++;
	}

	log_info(logger, "Thread CPU %d - Se reemplaza página nro %d, del proceso PID %d", socketCPU, nodoPagina->nro_pagina, nodoProceso->pid);
	//pthread_mutex_unlock(&mutexProcesos);
	if (bitModificado == 1) {
		void * pagina = lecturaMemoria(idFrame, 0, size_frames);
		enviarPaginaASwap(nodoPagina->nro_pagina, nodoProceso->pid, pagina);
		log_info(logger, "Thread CPU %d - Se envía a Swap página nro %d, del proceso PID %d", socketCPU, nodoPagina->nro_pagina, nodoProceso->pid);
	}

}

void limpiarEntradaTLBPorFrame(uint32_t nroFrame) {
	t_entrada_tlb* nodoAux;
	int i = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB)) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->nroFrame == nroFrame) {
			nodoAux->pid = 0;
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexTLB);

}

void algoritmoDeReemplazo(uint32_t pid, uint32_t paginaNueva,
		void * codigoPagina, int * idFrame, t_nodo_lista_procesos * nodoProceso, int socketCPU) {
	int indiceFrame;
	int bitModificado;
	t_nodo_lista_frames * frameAux;
	//t_nodo_lista_procesos * nodoProceso;
	//int indiceProceso;

	//pthread_mutex_lock(&mutexFrames);
	if (strcmp(algoritmo, "CLOCK") == 0) {
		indiceFrame = buscarVictimaClock(nodoProceso, pid);
	} else {
		indiceFrame = buscarVictimaClockModificado(nodoProceso, pid);
	}

	//indiceProceso = encontrarPosicionEnListaProcesos(pid); //carajooo, modificar esto. ojo con los retardos!!
	//nodoProceso = list_get(listaProcesos, indiceProceso);
	frameAux = list_get(listaFrames, indiceFrame);
	*idFrame = frameAux->nroFrame;
	bitModificado = frameAux->bitModificado;
	frameAux->bitModificado = 0;
	pthread_mutex_unlock(&mutexFrames);

	actualizarPaginaAReemplazar(nodoProceso, *idFrame, bitModificado, socketCPU); //cambia status de pagina anterior de 'M' a 'S'

	limpiarEntradaTLBPorFrame(*idFrame);

	actualizarNodoPaginaNuevaCargadaEnM(nodoProceso, *idFrame, paginaNueva); //cambia status de nueva pagina cargada en memoria

	escrituraMemoria(*idFrame, 0, size_frames, codigoPagina);

	log_info(logger,
			"Thread CPU %d - Se ejecutó algoritmo de reemplazo de páginas %s en memoria del proceso PID %d.", socketCPU, algoritmo,
			pid);
}

void actualizarBitUltimoAccesoTLB(uint32_t pid, int nroFrame) {
	t_entrada_tlb* nodoAux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB) && acierto == 0) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid && nodoAux->nroFrame == (uint32_t) nroFrame) {
			pthread_mutex_lock(&mutexContadorMemoria);
			nodoAux->ultAcceso = accesoMemoria;
			pthread_mutex_unlock(&mutexContadorMemoria);
			acierto = 1;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexTLB);

}

void cargarEnTLB(uint32_t pid, uint32_t nroPagina, uint32_t nroFrame, int socketCPU) {
	t_entrada_tlb* aux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB) && acierto == 0) {
		aux = list_get(TLB, i);
		if (aux->pid == 0) {
			aux->pid = pid;
			aux->nroFrame = nroFrame;
			aux->nroPagina = nroPagina;
			pthread_mutex_lock(&mutexContadorMemoria);
			aux->ultAcceso = accesoMemoria;
			pthread_mutex_unlock(&mutexContadorMemoria);
			acierto = 1;
		}
		i++;
	}

	if (acierto == 0) {
		lru(nroPagina, pid, nroFrame, socketCPU);
	}
	pthread_mutex_unlock(&mutexTLB);

	log_info(logger, "Thread CPU %d - Página nro %d, del proceso PID %d, cargada en TLB", socketCPU,
			nroPagina, pid);
}

int cargarPaginaEnMemoria(uint32_t pid, uint32_t nroPagina, void *buffer,
		int * idFrame, int socketCPU) {
	t_nodo_lista_frames* aux;
	t_nodo_lista_procesos * auxProceso;
	int i = 0, j = 0;
	int permitido = 0;

	pthread_mutex_lock(&mutexProcesos);
	pthread_mutex_lock(&mutexFrames);
	int disponible = cantidadFramesDisponibles();

	//Primero busco en la lista de procesos a ver si llego a su máximo de frames por proceso
	while (j < list_size(listaProcesos)) {
		auxProceso = list_get(listaProcesos, j);
		if ((auxProceso->pid) == pid) {
			if ((auxProceso->framesAsignados) < framesPorProceso
					&& (disponible != 0)) {
				permitido = 1;
			}
			break;
		}
		j++;
	}
	//pthread_mutex_unlock(&mutexProcesos);

	if ((disponible == 0) && ((auxProceso->framesAsignados) == 0)) {
		free(buffer);
		pthread_mutex_unlock(&mutexFrames);
		pthread_mutex_unlock(&mutexProcesos);
		return -1; //No se puede cargar página en memoria.
	}

	if (permitido == 1) {
		//pthread_mutex_lock(&mutexFrames);
		while (i < list_size(listaFrames)) {
			aux = list_get(listaFrames, i);
			if (aux->pid == 0) {
				aux->pid = pid;
				aux->bitModificado = 0;
				aux->bitReferencia = 1;
				*idFrame = aux->nroFrame;
				break;
			}
			i++;
		}
		//pthread_mutex_lock(&mutexProcesos);
		auxProceso->framesAsignados++;
		if (auxProceso->framesAsignados == 1) {
			auxProceso->punteroClock = i;
		}
		//pthread_mutex_unlock(&mutexProcesos);
		actualizarNodoPaginaNuevaCargadaEnM(auxProceso, *idFrame, nroPagina);
		pthread_mutex_unlock(&mutexFrames);
		pthread_mutex_unlock(&mutexProcesos);
		escrituraMemoria(*idFrame, 0, size_frames, buffer);

	} else {
		algoritmoDeReemplazo(pid, nroPagina, buffer, idFrame, auxProceso, socketCPU);
		pthread_mutex_unlock(&mutexProcesos);
	}

	log_info(logger, "Thread CPU %d - Se cargó en memoria página nro %d del proceso PID %d", socketCPU,
			nroPagina, pid);
	return 1; //Se logró cargar página en memoria
}

void recibirPaginaDeSwap(void * pagina) {
	int bytesRecibidos;
	bytesRecibidos = recibirTodo(socketSwap, pagina, size_frames);
	if (bytesRecibidos == 1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC.");
		abort();
	}
}

void enviarASwapFinalizarPrograma(uint32_t pid) {
	int header = finalizacionPrograma;
	int error;
	send(socketSwap, &header, sizeof(int), MSG_NOSIGNAL);
	error = send(socketSwap, &pid, sizeof(uint32_t), MSG_NOSIGNAL);
	if (error == -1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC.");
		abort();
	}

}

void finalizarPrograma(uint32_t idPrograma) { //creo que está terminada
	int indiceListaProcesos = encontrarPosicionEnListaProcesos(idPrograma);

	if (indiceListaProcesos >= 0) {
		pthread_mutex_lock(&mutexProcesos); //NO PONER RETARDO ACA PORQUE YA ESTA EN ENCONTRAR POS EN LISTA PROCESOS
		liberarPaginas(indiceListaProcesos);
		list_remove_and_destroy_element(listaProcesos, indiceListaProcesos,
				(void*) destruirProceso);
		pthread_mutex_unlock(&mutexProcesos);
	}

	liberarFrames(idPrograma); //pongo ids en cero

	if (entradasTLB > 0) {
		limpiarEntradasTLBporPID(idPrograma);
	}

	pthread_mutex_lock(&mutexSwap);
	enviarASwapFinalizarPrograma(idPrograma);
	pthread_mutex_unlock(&mutexSwap);

	log_info(logger, "Thread Nucleo - Se finalizó programa pid %d", idPrograma);

}

void enviarBytesACPU(int socketCPU, void * data, int tamanio) {
	int fallo = 0;
	fallo = send(socketCPU, data, tamanio, MSG_NOSIGNAL);
	if (fallo == -1) {
		pthread_t socketCPU = pthread_self();
		log_info(logger, "Thread %d - Se desconectó CPU", socketCPU);
		free(data);
		pthread_exit(NULL);
	}
	free(data);
}

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio,
		uint32_t pid, int socketCPU) { //ahora si posta posta creo que está terminada

	void * data;
	int nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			log_info(logger, "Thread CPU %d - TLB hit", socketCPU);
			data = lecturaMemoria(nroFrame, offset, tamanio);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			enviarPedidoMemoriaOK(socketCPU);
			enviarBytesACPU(socketCPU, data, tamanio);
			log_info(logger,
					"Thread CPU %d - Se enviaron a CPU %d bytes de la página %d, offset %d, del proceso PID %d.", socketCPU,
					tamanio, nroPagina, offset, pid);
			return;
		}
	}
	log_info(logger, "Thread CPU %d - TLB miss", socketCPU); //TLB Miss
	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) { //tengo que traer pagina de swap
		int exito;
		void * bufferPagina = malloc(size_frames);
		pthread_mutex_lock(&mutexSwap);
		pedirPaginaASwap(socketSwap, pid, nroPagina);
		recibirPaginaDeSwap(bufferPagina);
		pthread_mutex_unlock(&mutexSwap);
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina, &nroFrame, socketCPU);
		if (exito == -1) {
			finalizarPrograma(pid); //En finalizarPrograma se avisa a Swap para que borre las páginas
			enviarAbortarProceso(socketCPU); //Le avisa a CPU que finalice el programa
			log_error(logger,
					"Thread CPU %d - No se pudo cargar página en memoria del proceso PID %d. No hay frames disponibles.", socketCPU,
					pid);
			return;
		}

	} else if (nroFrame == OVERFLOW) {
		finalizarPrograma(pid);
		enviarAbortarProceso(socketCPU);
		log_error(logger,
				"Thread CPU %d - Pedido inválido del proceso PID %d. Fuera del espacio de direcciones.", socketCPU,
				pid);
		return;
	}

	data = lecturaMemoria(nroFrame, offset, tamanio);

	if (entradasTLB > 0) {
		cargarEnTLB(pid, nroPagina, nroFrame, socketCPU);
		//actualizarBitUltimoAccesoTLB(pid, nroFrame);
	}

	enviarPedidoMemoriaOK(socketCPU);
	enviarBytesACPU(socketCPU, data, tamanio);

	log_info(logger,
			"Thread CPU %d - Se enviaron a CPU %d bytes de la página %d, offset %d, del proceso PID %d.", socketCPU,
			tamanio, nroPagina, offset, pid);

}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio,
		void * buffer, uint32_t pid, int socketCPU) {

	int nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			log_info(logger, "Thread CPU %d - TLB hit", socketCPU);
			escrituraMemoria(nroFrame, offset, tamanio, buffer);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			actualizarBitModificado(nroFrame);
			enviarPedidoMemoriaOK(socketCPU);
			log_info(logger,
					"Thread CPU %d - Se almacenaron %d bytes en la página %d, offset %d, del proceso PID %d.", socketCPU,
					tamanio, nroPagina, offset, pid);
			return;
		}
	}
	log_info(logger, "Thread CPU %d - TLB miss", socketCPU);
	//TLB Miss
	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) {
		int exito;
		void * bufferPagina = malloc(size_frames);
		pthread_mutex_lock(&mutexSwap);
		pedirPaginaASwap(socketSwap, pid, nroPagina);
		recibirPaginaDeSwap(bufferPagina);
		pthread_mutex_unlock(&mutexSwap);
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina, &nroFrame, socketCPU);
		if (exito == -1) {
			enviarAbortarProceso(socketCPU); //Le avisa a CPU que finalice el programa
			finalizarPrograma(pid); //En finalizarPrograma se avisa a Swap para que borre las páginas
			log_error(logger,
					"Thread CPU %d - No se pudo cargar página en memoria del proceso PID %d. No hay frames disponibles.", socketCPU,
					pid);
			return;
		}

	} else if (nroFrame == OVERFLOW) {
		enviarAbortarProceso(socketCPU);
		finalizarPrograma(pid);
		log_error(logger,
				"Thread CPU %d - Pedido inválido del proceso PID %d. Fuera del espacio de direcciones.", socketCPU,
				pid);
		return;
	}

	escrituraMemoria(nroFrame, offset, tamanio, buffer);
	actualizarBitModificado(nroFrame);

	if (entradasTLB > 0) {
		cargarEnTLB(pid, nroPagina, nroFrame, socketCPU);
		//actualizarBitUltimoAccesoTLB(pid, nroFrame);
	}

	enviarPedidoMemoriaOK(socketCPU);
	log_info(logger,
			"Thread CPU %d - Se almacenaron %d bytes en la página %d, offset %d, del proceso PID %d.", socketCPU,
			tamanio, nroPagina, offset, pid);

}

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma, int socketNucleo) { //creo que está terminada

	int fallo = 0;

	int largoPrograma = strlen(codigoPrograma) + 1; //?

	int paginasCodigo = largoPrograma / size_frames;
	if ((largoPrograma % size_frames) != 0) {
		paginasCodigo++;
	}

	int respuestaInicializacion, j, i;

	pthread_mutex_lock(&mutexSwap);
	enviarPaginasRequeridasASwap(socketSwap, paginasRequeridas);
	respuestaInicializacion = recibirRespuestaInicializacion(socketSwap);
	if (respuestaInicializacion == -1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC");
		abort();
	}
	log_info(logger, "Thread Nucleo - Se envió nuevo programa pid %d a Swap", idPrograma);

	if (respuestaInicializacion == inicioProgramaExito) {
		send(socketSwap, &idPrograma, sizeof(uint32_t), MSG_NOSIGNAL); //envio ID a Swap
		send(socketSwap, &paginasCodigo, sizeof(int), MSG_NOSIGNAL); //Juan: Faltaba enviar cant pags codigo a Swap
		char * pagina = malloc(size_frames);
		char * posicionAux = codigoPrograma; //CHEQUEAR ESTO DE LA POSICION AUXILIAR

		for (j = 0; j < paginasCodigo; j++) { //Le mando pagina por pagina a Swap
			if (largoPrograma >= size_frames) {
				memcpy(pagina, posicionAux, size_frames); //chequear si en la ultima pagina tira error
			} else {
				memcpy(pagina, posicionAux, largoPrograma);
			}

			fallo = send(socketSwap, pagina, size_frames, MSG_NOSIGNAL); //ver si se puede cambiar por enviarPaginaASwap
			posicionAux += size_frames;
			largoPrograma -= size_frames;
		}

		if (fallo == -1) {
			log_error(logger, "Se desconectó Swap. Abortando UMC.");
			abort();

		}

		free(pagina);
		enviarRespuestaInicializacionExito(socketNucleo);

	} else {
		enviarRespuestaInicializacionError(socketNucleo);
		log_info(logger,
				"Thread Nucleo - No se pudo inicializar programa PID %d. No hay espacio en Swap",
				idPrograma);
		pthread_mutex_unlock(&mutexSwap);
		return;
	}
	pthread_mutex_unlock(&mutexSwap);

	t_nodo_lista_procesos* unNodo = malloc(sizeof(t_nodo_lista_procesos));
	unNodo->pid = idPrograma;
	unNodo->cantPaginas = paginasRequeridas;
	unNodo->framesAsignados = 0;
	//unNodo->punteroClock = -1;
	unNodo->lista_paginas = list_create();

	for (i = 0; i < paginasRequeridas; i++) {
		t_nodo_lista_paginas* unaPagina = malloc(sizeof(t_nodo_lista_paginas));
		unaPagina->nro_pagina = i;
		unaPagina->status = 'S';
		unaPagina->nroFrame = cant_frames + 1;
		list_add(unNodo->lista_paginas, unaPagina);
	}

	pthread_mutex_lock(&mutexProcesos);
	list_add(listaProcesos, unNodo);
	pthread_mutex_unlock(&mutexProcesos);

	log_info(logger,
			"Thread Nucleo - Se inicializó nuevo programa PID %d. Cantidad de páginas totales: %d",
			idPrograma, paginasRequeridas);

	free(codigoPrograma);

}

void cambioProceso(uint32_t idNuevoPrograma, uint32_t * idProcesoActivo) { //ya tiene logger
	if (*idProcesoActivo != idNuevoPrograma) {

		if (entradasTLB > 0) {
			limpiarEntradasTLBporPID(*idProcesoActivo);
		}

	}

	(*idProcesoActivo) = idNuevoPrograma;

}

void cambiarRetardo(int nuevoRetardo) { //ya tiene logger
	retardo = nuevoRetardo;
}

void dumpEstructuraPaginas(t_nodo_lista_procesos* nodoAux, FILE* archivo) {
	int i = 0;
	t_nodo_lista_paginas*nodoAuxPagina;
	printf("Numero de Pagina\tEstado\n");
	fprintf(archivo, "Numero de Pagina\tEstado\n");
	while (i < list_size(nodoAux->lista_paginas)) { //NO PONER MUTEX ACA
		nodoAuxPagina = list_get(nodoAux->lista_paginas, i);
		printf("%d               \t%c\n", nodoAuxPagina->nro_pagina,
				nodoAuxPagina->status);
		fprintf(archivo, "%d               \t%c\n", nodoAuxPagina->nro_pagina,
				nodoAuxPagina->status);
		i++;
	}
}

void dumpPIDAuxiliar(t_nodo_lista_procesos*nodoAux, FILE*archivo) {
	fprintf(archivo,
			"PID\tCansocketCPUad de Frames Asignados\tCansocketCPUad de Paginas\n");
	printf("PID\tCansocketCPUad de Frames Asignados\tCansocketCPUad de Paginas\n");
	printf("%d\t%d                            \t%d\n", nodoAux->pid,
			nodoAux->framesAsignados, nodoAux->cantPaginas);
	fprintf(archivo, "%d\t%d                            \t%d\n", nodoAux->pid,
			nodoAux->framesAsignados, nodoAux->cantPaginas);
	dumpEstructuraPaginas(nodoAux, archivo);
}

void dumpTodosLosProcesos() {
	FILE*archivo = fopen("dumpUMC.txt", "w+");
	int i = 0;
	t_nodo_lista_procesos*nodoAux;
	pthread_mutex_lock(&mutexProcesos);
	while (i < list_size(listaProcesos)) {
		nodoAux = list_get(listaProcesos, i);
		dumpPIDAuxiliar(nodoAux, archivo);
		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);

	fprintf(archivo, "\nDump de Memoria Principal\n");
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	hexdump(archivo, memoriaPrincipal, tamanioMemoria);
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	fclose(archivo);
}

void dumpMemoriaPID(t_nodo_lista_procesos* nodoAux, FILE*archivo) {
	int i = 0;
	t_nodo_lista_paginas* nodoAuxPagina;
	//printf("\nDump memoria del PID:\n");
	fprintf(archivo, "\nDump memoria del PID:\n");
	pthread_mutex_lock(&mutexProcesos);
	while (i < list_size(nodoAux->lista_paginas)) {
		nodoAuxPagina = list_get(nodoAux->lista_paginas, i);
		if (nodoAuxPagina->status == 'M') {
			void*buffer = lecturaMemoria(nodoAuxPagina->nroFrame, 0,
					size_frames);
			hexdump(archivo, buffer, size_frames);
			free(buffer);
		}
		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);
}

void dumpPID(uint32_t pid) {
	int indiceProceso = encontrarPosicionEnListaProcesos(pid);

	if (indiceProceso == -1) {
		printf("PID no valido\n");
		return;
	}

	FILE* reporte = fopen("dumpPID.txt", "w+");
	fprintf(reporte,
			"PID\tCansocketCPUad de Frames Asignados\tCansocketCPUad de Paginas\n");
	printf("PID\tCansocketCPUad de Frames Asignados\tCansocketCPUad de Paginas\n");

	pthread_mutex_lock(&mutexProcesos);
	t_nodo_lista_procesos*nodoAux = list_get(listaProcesos, indiceProceso);
	pthread_mutex_unlock(&mutexProcesos);

	printf("%d\t%d                            \t%d\n", pid,
			nodoAux->framesAsignados, nodoAux->cantPaginas);
	fprintf(reporte, "%d\t%d                            \t%d\n", pid,
			nodoAux->framesAsignados, nodoAux->cantPaginas);

	dumpEstructuraPaginas(nodoAux, reporte);
	dumpMemoriaPID(nodoAux, reporte);

	fclose(reporte);

}

int clean_stdin() {
	while (getchar() != '\n')
		;
	return 1;
}

void consolaUMC(void) {
	char * comando = malloc(30);
	char c;
	int nuevoRetardo;
	int pid;

	while (1) {
		printf("Ingrese comando:\n");
		scanf("%s", comando);
		if (strncasecmp(comando, "flush", 5) == 0) {
			printf("¿Sobre que quiere hacer flush?\n-tlb\n-memory");
			scanf("%s", comando);

			if (strncasecmp(comando, "tlb", 3) == 0) {
				if (entradasTLB > 0) {
					printf("Se ejecutará: flush TLB\n");
					flushTLB();
					log_info(logger, "Se ejecutó flush TLB\n");

				} else {
					printf(
							"No se puede ejecutar flush TLB porque no tiene entradas\n");

				}
			} else if (strncasecmp(comando, "memory", 6) == 0) {
				printf("Se ejecutará: Flush Memoria Principal\n");
				flushMemory();
				log_info(logger, "Se ejecutó flush Memory");
			}
		} else if (strncasecmp(comando, "dump", 4) == 0) {
			printf("¿Sobre qué quiere hacer dump?\n -total \n -pid \n");
			scanf("%s", comando);
			if (strncasecmp(comando, "pid", 3) == 0) {
				do {
					printf("Ingrese PID: \n");

				} while ((scanf("%d%c", &pid, &c) != 2 || c != '\n')
						&& clean_stdin());

				printf("Se ejecutará: Dump del proceso pid %d\n", pid);

				dumpPID(pid);
			} else if (strncasecmp(comando, "total", 5) == 0) {
				printf("Se ejecutará: Dump de todos los procesos\n");
				dumpTodosLosProcesos();
			}

		} else if (strncasecmp(comando, "retardo", 7) == 0) {
			do {
				printf("Ingrese nuevo retardo: \n");

			} while ((scanf("%d%c", &nuevoRetardo, &c) != 2 || c != '\n')
					&& clean_stdin());

			printf("Se ejecutará: Cambio de retardo\n");
			cambiarRetardo(nuevoRetardo);
			log_info(logger,
					"Se ejecutó cambio de retardo. El retardo ahora es %d",
					retardo);
		} else {
			printf("Comando no válido.\n");
		}
	}

}

void procesarOperacionesNucleo(int * conexion) {
	int socketNucleo = *conexion;
	free(conexion);

	while (1) {

		int header = recibirHeader(socketNucleo);
		uint32_t pid;
		int largo_codigo, respuesta;
		uint32_t paginas_requeridas;
		char * programa;

		switch (header) {

		case 0:
			log_error(logger, "Se desconectó Núcleo");
			abort();
			break;
		case iniciarPrograma:
			respuesta = recibirInicializacionPrograma(socketNucleo, &pid,
					&paginas_requeridas, &largo_codigo);
			if (respuesta == -1) {
				log_error(logger, "Se desconectó Núcleo");
				abort();
			}
			programa = malloc(largo_codigo);
			respuesta = recibirCodigoInicializarPrograma(socketNucleo,
					largo_codigo, programa);
			if (respuesta == -1) {
				log_error(logger, "Se desconectó Núcleo");
				abort();
			}
			inicializarPrograma(pid, paginas_requeridas, programa,
					socketNucleo);
			break;
		case finalizacionPrograma:
			respuesta = recibirPID(socketNucleo, &pid);
			if (respuesta == -1) {
				log_error(logger, "Se desconectó Núcleo");
				abort();
			}
			finalizarPrograma(pid);
			break;
		default:
			log_error(logger, "Hubo un problema de conexión con Núcleo");
			abort();

		}

	}
}

void procesarSolicitudOperacionCPU(int * socketCPU) {

	uint32_t idCambioProceso = 0;
	int conexion = *socketCPU;
	free(socketCPU);

	while (1) {
		int header = recibirHeader(conexion);

		uint32_t nroPagina;
		uint32_t offset;
		uint32_t size;
		void * bufferPedido;
		uint32_t idNuevoProcesoActivo;

		switch (header) {
		case 0:
			log_info(logger, "Thread CPU %d - Se desconectó CPU", conexion);
			pthread_exit(NULL);
			break;

		case solicitarBytes:

			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size);
			solicitarBytesDeUnaPag(nroPagina, offset, size, idCambioProceso,
					conexion);
			break;

		case almacenarBytes:
			recibirPedidoAlmacenarBytes(conexion, &nroPagina, &offset, &size);

			bufferPedido = malloc(size);
			recibirBufferPedidoAlmacenarBytes(conexion, size, bufferPedido);
			almacenarBytesEnUnaPag(nroPagina, offset, size, bufferPedido,
					idNuevoProcesoActivo, conexion);
			break;
		case cambiarProcesoActivo:
			recibirPID(conexion, &idNuevoProcesoActivo);
			log_info(logger,
					"Thread CPU %d - Cambio proceso activo. PID proceso anterior: %d. PID nuevo proceso activo: %d.",
					conexion, idCambioProceso, idNuevoProcesoActivo);
			cambioProceso(idNuevoProcesoActivo, &idCambioProceso);

			break;

		default:
			log_error(logger, "Thread CPU %d - Hubo problema de conexion con CPU", conexion);
			pthread_exit(NULL);

			break;
		}
	}

}

