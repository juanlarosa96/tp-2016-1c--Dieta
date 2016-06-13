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
	pthread_mutex_lock(&mutexFrames);
	while (i < list_size(listaFrames)) {
		nodoAux = list_get(listaFrames, i);
		if (nodoAux->pid == 0) {
			contador++;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexFrames);
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
		if ((nodoAux->pid == pid) && (nodoAux->nroPagina == (uint32_t) nroPagina)) {
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

void lru(int paginaNueva, uint32_t pid, uint32_t frame) { //antes de llamar a lru mutex TLB

	int indiceVictima = buscarEntradaMenosUsadaRecientemente();

	t_entrada_tlb* entradaAuxiliar;

	entradaAuxiliar = list_get(TLB, indiceVictima);
	entradaAuxiliar->nroPagina = paginaNueva;
	entradaAuxiliar->pid = pid;
	entradaAuxiliar->nroFrame = frame;
	pthread_mutex_lock(&mutexContadorMemoria);
	entradaAuxiliar->ultAcceso = accesoMemoria; //OJO VARIABLE GLOBAL
	pthread_mutex_unlock(&mutexContadorMemoria);
	/*list_replace_and_destroy_element(TLB, indiceVictima, entradaAuxiliar,
	 (void *) entradaTLBdestroy);*/

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

void limpiarEntradasTLB(uint32_t pid) {
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
	int i = 0;
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
	int i = 0;
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

int buscarVictimaClock(uint32_t pid) {
	int indice;
	int puntero;
	int nuevoPuntero;
	int victima;
	int acierto;
	t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_frames* nodoFrame;
	indice = encontrarPosicionEnListaProcesos(pid);

	pthread_mutex_lock(&mutexProcesos);
	nodoAux = list_get(listaProcesos, indice);
	puntero = nodoAux->punteroClock;
	pthread_mutex_unlock(&mutexProcesos);

	pthread_mutex_lock(&mutexFrames);
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
	pthread_mutex_unlock(&mutexFrames);

	return victima;

}

int buscarVictimaClockModificado(uint32_t pid) {
	int indice;
	int puntero, punteroInicial;
	int nuevoPuntero;
	int victima;
	int acierto = 0;
	int cantVueltas = 0;
	t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_frames* nodoFrame;
	indice = encontrarPosicionEnListaProcesos(pid);

	pthread_mutex_lock(&mutexProcesos);
	nodoAux = list_get(listaProcesos, indice);
	puntero = nodoAux->punteroClock;
	pthread_mutex_unlock(&mutexProcesos);
	punteroInicial = puntero;

	pthread_mutex_lock(&mutexFrames);
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

	pthread_mutex_unlock(&mutexFrames);

	return victima;

}

void actualizarNodoPaginaNuevaCargadaEnM(int indiceProceso, uint32_t idFrame,
		uint32_t paginaNueva) {
	t_nodo_lista_procesos * nodoProceso;
	t_nodo_lista_paginas * nodoPagina;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexProcesos);
	nodoProceso = list_get(listaProcesos, indiceProceso);

	while (i < list_size(nodoProceso->lista_paginas) && acierto == 0) {
		nodoPagina = list_get(nodoProceso->lista_paginas, i);
		if (nodoPagina->nro_pagina == paginaNueva) {
			nodoPagina->nroFrame = idFrame;
			nodoPagina->status = 'M';
			acierto = 1;
		}

		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);

}

void enviarPaginaASwap(int nroPagina, uint32_t pid, void * pagina) {
	int header = guardarPaginasEnSwap;
	void * data = malloc(sizeof(int) * 2 + sizeof(uint32_t) + size_frames);
	int offset = 0;
	memcpy(data, &header, sizeof(int));
	offset += sizeof(int);
	memcpy(data + offset, &nroPagina, sizeof(int));
	offset += sizeof(int);
	memcpy(data + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(data + offset, pagina, size_frames);
	offset += size_frames;
	pthread_mutex_lock(&mutexSwap);
	send(socketSwap, data, offset, 0);
	pthread_mutex_unlock(&mutexSwap);
	free(pagina);
	free(data);
}

void actualizarPaginaAReemplazar(int indiceProceso, int idFrame,
		int bitModificado) {
	t_nodo_lista_procesos * nodoProceso;
	t_nodo_lista_paginas * nodoPagina;
	int i = 0;

	pthread_mutex_lock(&mutexProcesos);
	nodoProceso = list_get(listaProcesos, indiceProceso);

	while (i < list_size(nodoProceso->lista_paginas)) {
		nodoPagina = list_get(nodoProceso->lista_paginas, i);
		if (nodoPagina->nroFrame == (uint32_t) idFrame) {
			nodoPagina->status = 'S';
			break;
		}

		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);
	void * pagina = lecturaMemoria(idFrame, 0, size_frames);
	if (bitModificado == 1) {
		enviarPaginaASwap(nodoPagina->nro_pagina, nodoProceso->pid, pagina);
	}

}

void algoritmoDeReemplazo(uint32_t pid, uint32_t paginaNueva,
		void * codigoPagina) {
	int indiceFrame;
	int idFrame, bitModificado;
	t_nodo_lista_frames * frameAux;
	int indiceProceso;

	if (strncasecmp(algoritmo, "CLOCK", 5) == 0) {
		indiceFrame = buscarVictimaClock(pid);
	} else {
		indiceFrame = buscarVictimaClockModificado(pid);
	}

	indiceProceso = encontrarPosicionEnListaProcesos(pid); //carajooo, modificar esto. ojo con los retardos!!

	pthread_mutex_lock(&mutexFrames);
	frameAux = list_get(listaFrames, indiceFrame);
	idFrame = frameAux->nroFrame;
	bitModificado = frameAux->bitModificado;
	frameAux->bitModificado = 0;
	pthread_mutex_unlock(&mutexFrames);

	actualizarPaginaAReemplazar(indiceProceso, idFrame, bitModificado); //cambia status de pagina anterior de 'M' a 'S'

	actualizarNodoPaginaNuevaCargadaEnM(indiceProceso, idFrame, paginaNueva); //cambia status de nueva pagina cargada en memoria

	escrituraMemoria(idFrame, 0, size_frames, codigoPagina);
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

void cargarEnTLB(uint32_t pid, uint32_t nroPagina, uint32_t nroFrame) {
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
		lru(nroPagina, pid, nroFrame);
	}
	pthread_mutex_unlock(&mutexTLB);

}

int cargarPaginaEnMemoria(uint32_t pid, uint32_t nroPagina, void *buffer) {
	t_nodo_lista_frames* aux;
	t_nodo_lista_procesos * auxProceso;
	int i = 0, j = 0;
	int  permitido = 0, disponible = 0;
	uint32_t idFrame;

	//Primero busco en la lista de procesos a ver si llego a su máximo de frames por proceso
	pthread_mutex_lock(&mutexProcesos);
	while (j < list_size(listaProcesos)) {
		auxProceso = list_get(listaProcesos, j);
		if ((auxProceso->pid) == pid) {
			if ((auxProceso->framesAsignados) < framesPorProceso) {
				permitido = 1;
				if ((auxProceso->framesAsignados) == 0) {
					disponible = cantidadFramesDisponibles();
				}
			}
			break;
		}
		j++;
	}
	pthread_mutex_unlock(&mutexProcesos);

	if ((disponible == 0) && ((auxProceso->framesAsignados) == 0)) {
		free(buffer);
		return -1; //No se puede cargar página en memoria.
	}

	if (permitido == 1) {
		pthread_mutex_lock(&mutexFrames);
		while (i < list_size(listaFrames)) {
			aux = list_get(listaFrames, i);
			if (aux->pid == 0) {
				aux->pid = pid;
				aux->bitModificado = 0;
				aux->bitReferencia = 1;
				idFrame = aux->nroFrame;
				break;
			}
			i++;
		}
		pthread_mutex_unlock(&mutexFrames);
		pthread_mutex_lock(&mutexProcesos);
		auxProceso->framesAsignados ++;
		if(auxProceso->framesAsignados == 1){
			auxProceso->punteroClock = i;
		}
		pthread_mutex_unlock(&mutexProcesos);
		actualizarNodoPaginaNuevaCargadaEnM(j, idFrame, nroPagina);
		escrituraMemoria(idFrame, 0, size_frames, buffer);

	} else {
		algoritmoDeReemplazo(pid, nroPagina, buffer);
	}

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
	error = send(socketSwap, &header, sizeof(int), 0);
	if (error == -1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC.");
		abort();
	}

	error = send(socketSwap, &pid, sizeof(uint32_t), 0);
	if (error == -1) {
		log_error(logger, "Se desconectó Swap. Abortando UMC.");
		abort();
	}

}

void finalizarPrograma(uint32_t idPrograma) { //creo que está terminada
	int indiceListaProcesos = encontrarPosicionEnListaProcesos(idPrograma);

	pthread_mutex_lock(&mutexProcesos); //NO PONER RETARDO ACA PORQUE YA ESTA EN ENCONTRAR POS EN LISTA PROCESOS
	liberarPaginas(indiceListaProcesos);
	list_remove_and_destroy_element(listaProcesos, indiceListaProcesos,
			(void*) destruirProceso);
	pthread_mutex_unlock(&mutexProcesos);

	liberarFrames(idPrograma); //pongo ids en cero

	if (entradasTLB > 0) {
		limpiarEntradasTLB(idPrograma);
	}

	pthread_mutex_lock(&mutexSwap);
	enviarASwapFinalizarPrograma(idPrograma);
	pthread_mutex_unlock(&mutexSwap);

	log_info(logger, "Se finalizó programa pid %d", idPrograma);

}

void enviarBytesACPU(int socketCPU, void * data, int tamanio) {
	send(socketCPU, data, tamanio, 0);
	free(data);
}

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio,
		uint32_t pid, int socketCPU) { //ahora si posta posta creo que está terminada

	void * data;
	int nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			data = lecturaMemoria(nroFrame, offset, tamanio);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			enviarBytesACPU(socketCPU, data, tamanio);
			return;
		}
	}
	//TLB Miss
	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) { //tengo que traer pagina de swap
		int exito;
		void * bufferPagina = malloc(size_frames);
		pthread_mutex_lock(&mutexSwap);
		pedirPaginaASwap(socketSwap, pid, nroPagina);
		recibirPaginaDeSwap(bufferPagina);
		pthread_mutex_unlock(&mutexSwap);
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina);
		if (exito == -1) {
			finalizarPrograma(pid); //En finalizarPrograma se avisa a Swap para que borre las páginas
			enviarAbortarProceso(socketCPU); //Le avisa a CPU que finalice el programa
			log_error(logger,
					"No se puede cargar página en memoria del proceso pid %d. No hay frames disponibles.",
					pid);
			return;
		}

	} else if (nroFrame == OVERFLOW) {
		finalizarPrograma(pid);
		enviarAbortarProceso(socketCPU);
		log_error(logger,
				"Pedido inválido del proceso pid %d. Fuera del espacio de direcciones.",
				pid);
		return;
	}

	data = lecturaMemoria(nroFrame, offset, tamanio);

	if (entradasTLB > 0) {
		cargarEnTLB(pid, nroPagina, nroFrame);
		actualizarBitUltimoAccesoTLB(pid, nroFrame);
	}

	enviarPedidoMemoriaOK(socketCPU);
	enviarBytesACPU(socketCPU, data, tamanio);

	log_info(logger,
			"Se enviaron a CPU %d bytes de la página %d, offset %d, del proceso %d.",
			tamanio, nroPagina, offset, pid);

}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio,
		void * buffer, uint32_t pid, int socketCPU) {

	int nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			escrituraMemoria(nroFrame, offset, tamanio, buffer);
			actualizarBitModificado(nroFrame);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			return;
		}
	}

	//TLB Miss
	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) {
		int exito;
		void * bufferPagina = malloc(size_frames);
		pthread_mutex_lock(&mutexSwap);
		pedirPaginaASwap(socketSwap, pid, nroPagina);
		recibirPaginaDeSwap(bufferPagina);
		pthread_mutex_unlock(&mutexSwap);
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina);
		if (exito == -1) {
			enviarAbortarProceso(socketCPU); //Le avisa a CPU que finalice el programa
			finalizarPrograma(pid); //En finalizarPrograma se avisa a Swap para que borre las páginas
			log_error(logger,
					"No se puede cargar página en memoria del proceso pid %d. No hay frames disponibles.",
					pid);
			return;
		}

	} else if (nroFrame == OVERFLOW) {
		enviarAbortarProceso(socketCPU);
		finalizarPrograma(pid);
		log_error(logger,
				"Pedido inválido del proceso pid %d. Fuera del espacio de direcciones.",
				pid);
		return;
	}

	escrituraMemoria(nroFrame, offset, tamanio, buffer);
	actualizarBitModificado(nroFrame);

	if (entradasTLB > 0) {
		cargarEnTLB(pid, nroPagina, nroFrame);
		actualizarBitUltimoAccesoTLB(pid, nroFrame);
	}

	enviarPedidoMemoriaOK(socketCPU);
	log_info(logger,
			"Se almacenaron %d bytes en la página %d, offset %d, del proceso %d.",
			tamanio, nroPagina, offset, pid);

}

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma, int socketNucleo) { //creo que está terminada

	int largoPrograma = strlen(codigoPrograma) + 1; //?

	int paginasCodigo = largoPrograma / size_frames;
	if ((largoPrograma % size_frames) != 0) {
		paginasCodigo++;
	}

	int respuestaInicializacion, j, i;

	pthread_mutex_lock(&mutexSwap);
	enviarPaginasRequeridasASwap(socketSwap, paginasRequeridas);
	respuestaInicializacion = recibirRespuestaInicializacion(socketSwap);
	log_info(logger, "Se envió nuevo programa pid %d a Swap", idPrograma);

	if (respuestaInicializacion == inicioProgramaExito) {
		send(socketSwap, &idPrograma, sizeof(uint32_t), 0); //envio ID a Swap
		send(socketSwap, &paginasCodigo, sizeof(int), 0); //Juan: Faltaba enviar cant pags codigo a Swap
		char * pagina = malloc(size_frames);
		char * posicionAux = codigoPrograma; //CHEQUEAR ESTO DE LA POSICION AUXILIAR

		for (j = 0; j < paginasCodigo; j++) { //Le mando pagina por pagina a Swap
			if (largoPrograma >= size_frames) {
				memcpy(pagina, posicionAux, size_frames); //chequear si en la ultima pagina tira error
			} else {
				memcpy(pagina, posicionAux, largoPrograma);
			}

			send(socketSwap, pagina, size_frames, 0); //ver si se puede cambiar por enviarPaginaASwap
			posicionAux += size_frames;
			largoPrograma -= size_frames;
		}

		free(pagina);
		enviarRespuestaInicializacionExito(socketNucleo);

	} else {
		enviarRespuestaInicializacionError(socketNucleo);
		log_info(logger,
				"No se pudo inicializar programa pid %d. No hay espacio en Swap",
				idPrograma);
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

	log_info(logger, "Se inicializó nuevo programa pid %d", idPrograma);

	free(codigoPrograma);

}

void cambioProceso(uint32_t idNuevoPrograma, uint32_t * idProcesoActivo) { //ya tiene logger

	if (entradasTLB > 0) {
		limpiarEntradasTLB(*idProcesoActivo);
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
	pthread_mutex_lock(&mutexProcesos);
	while (i < list_size(nodoAux->lista_paginas)) {
		nodoAuxPagina = list_get(nodoAux->lista_paginas, i);
		printf("%d               \t%c\n", nodoAuxPagina->nro_pagina,
				nodoAuxPagina->status);
		fprintf(archivo, "%d               \t%c\n", nodoAuxPagina->nro_pagina,
				nodoAuxPagina->status);
		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);
}

void dumpPIDAuxiliar(t_nodo_lista_procesos*nodoAux, FILE*archivo) {
	fprintf(archivo,
			"PID\tCantidad de Frames Asignados\tCantidad de Paginas\n");
	printf("PID\tCantidad de Frames Asignados\tCantidad de Paginas\n");
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

	printf("\nDump de Memoria Principal\n");
	fprintf(archivo, "\nDump de Memoria Principal\n");
	pthread_mutex_lock(&mutexMemoriaPrincipal);
	hexdump(archivo, memoriaPrincipal, tamanioMemoria);
	pthread_mutex_unlock(&mutexMemoriaPrincipal);
	fclose(archivo);
}

void dumpMemoriaPID(t_nodo_lista_procesos* nodoAux, FILE*archivo) {
	int i = 0;
	t_nodo_lista_paginas* nodoAuxPagina;
	printf("\nDump memoria del PID:\n");
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
			"PID\tCantidad de Frames Asignados\tCantidad de Paginas\n");
	printf("PID\tCantidad de Frames Asignados\tCantidad de Paginas\n");

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
				printf("Se ejecutará: flush TLB\n");
				flushTLB();
				log_info(logger, "Se ejecutó flush TLB\n");
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
		int largo_codigo;
		uint32_t paginas_requeridas;
		char * programa;

		switch (header) {

		case 0:
			log_error(logger, "Se desconectó Núcleo");
			abort();
			break;
		case iniciarPrograma:

			recibirInicializacionPrograma(socketNucleo, &pid,
					&paginas_requeridas, &largo_codigo);
			programa = malloc(largo_codigo);
			recibirCodigoInicializarPrograma(socketNucleo, largo_codigo,
					programa);
			inicializarPrograma(pid, paginas_requeridas, programa,
					socketNucleo);
			break;
		case finalizacionPrograma:
			recibirPID(socketNucleo, &pid);
			finalizarPrograma(pid);
			break;
		default:
			log_error(logger, "Hubo un problema de conexión con Núcleo");
			abort();

		}

	}
}

void procesarSolicitudOperacionCPU(int * socketCPU) {

	uint32_t idCambioProceso;
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

			log_info(logger, "Se desconectó CPU (socket %d)", conexion);
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
					"Se cambió el proceso activo de una CPU (socket %d). PID proceso anterior: %d. PID nuevo proceso activo:%d",
					conexion, idCambioProceso, idNuevoProcesoActivo);
			cambioProceso(idNuevoProcesoActivo, &idCambioProceso);

			break;

		default:
			log_error(logger, "Hubo problema de conexion con CPU (socket %d)",
					conexion);
			pthread_exit(NULL);

			break;
		}
	}

}

