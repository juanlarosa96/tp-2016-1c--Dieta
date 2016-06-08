/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

#define OVERFLOW -2

void cambiarRetardo(int nuevoRetardo) {
	retardo = nuevoRetardo;
}

void destruirProceso(t_nodo_lista_procesos * nodo) {
	free(nodo);
}

void destruirFrame(t_nodo_lista_frames * nodo) {
	free(nodo);
}

void destruirPagina(t_nodo_lista_paginas * nodo) {
	free(nodo);
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
	}
	pthread_mutex_unlock(&mutexFrames);
	return contador;
}

void inicializarPuntero(uint32_t pid, int posicionFrameLista) { //idFrame
	int indice = 0;
	t_nodo_lista_procesos* nodoAux;
	indice = encontrarPosicionEnListaProcesos(pid);
	pthread_mutex_lock(&mutexProcesos);
	nodoAux = list_get(listaProcesos, indice);
	nodoAux->punteroClock = posicionFrameLista;
//list_replace_and_destroy_element(listaProcesos, indice, nodoAux, (void*) destruirProceso);
	pthread_mutex_unlock(&mutexProcesos);

}

int buscarEnTLB(uint32_t pid, int nroPagina) {
	int frame = -1;

	t_entrada_tlb* nodoAux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB) && acierto == 0) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid && nodoAux->nroPagina == (uint32_t) nroPagina) {
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

	if(paginaEncontrada == 0){
		return OVERFLOW;
	}

	return (int) frame;
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
	actualizarBitModificado(frame);
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
void modificarFrameEnListaPaginas(int indiceProceso, uint32_t idFrame,
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

int buscarVictimaClockModificado(uint32_t pid) {
	int indice;
	int puntero;
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

			if (cantVueltas == 0) {
				cantVueltas++;
			} else {
				cantVueltas = 0;
			}
		}

	}
	pthread_mutex_unlock(&mutexFrames);

	return victima;

}

void actualizarPaginaAReemplazar(int indiceProceso, int idFrame) {
	t_nodo_lista_procesos * nodoProceso;
	t_nodo_lista_paginas * nodoPagina;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexProcesos);
	nodoProceso = list_get(listaProcesos, indiceProceso);

	while (i < list_size(nodoProceso->lista_paginas) && acierto == 0) {
		nodoPagina = list_get(nodoProceso->lista_paginas, i);
		if (nodoPagina->nroFrame == (uint32_t) idFrame) {
			nodoPagina->status = 'S';
			acierto = 1;
		}

		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);

}

void algoritmoDeReemplazo(uint32_t pid, uint32_t paginaNueva,
		void * codigoPagina) {
	int indiceFrame;
	int idFrame;
	t_nodo_lista_frames * frameAux;
	int indiceProceso;

	if (strncasecmp(algoritmo, "CLOCK", 5) == 0) {
		indiceFrame = buscarVictimaClock(pid);
	} else if (strncasecmp(algoritmo, "CLOCK-M", 7) == 0) {
		indiceFrame = buscarVictimaClockModificado(pid);
	} else {
		log_error(logger, "Algoritmo Desconocido. Abortando UMC");
		abort();
	}

	pthread_mutex_lock(&mutexFrames);
	frameAux = list_get(listaFrames, indiceFrame);
	idFrame = frameAux->nroFrame;
	if (frameAux->bitModificado == 1) {
		//enviarPaginaASwap
	}
	pthread_mutex_unlock(&mutexFrames);

	indiceProceso = encontrarPosicionEnListaProcesos(pid);

	actualizarPaginaAReemplazar(indiceProceso, idFrame); //cambia status de pagina anterior de 'M' a 'S'

	modificarFrameEnListaPaginas(indiceProceso, idFrame, paginaNueva); //cambia status de nueva pagina cargada en memoria

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
	int acierto = 0, aciertoAux = 0, permitido = 0, disponible = 0;
	uint32_t idFrame;

	//Primero busco en la lista de procesos a ver si llego a su máximo de frames por proceso
	pthread_mutex_lock(&mutexProcesos);
	while (j < list_size(listaProcesos) && aciertoAux == 0) {
		auxProceso = list_get(listaProcesos, j);
		if (auxProceso->pid == pid) {
			if ((auxProceso->framesAsignados) < framesPorProceso) {
				permitido = 1;
				if (auxProceso->framesAsignados == 0) {
					// disponible = hayFramesDisponibles();
				}
			}
			aciertoAux = 1;
		}
		j++;
	}
	pthread_mutex_lock(&mutexProcesos);

	if (disponible == 0 && auxProceso->framesAsignados == 0) {
		return -1; //No se puede cargar página en memoria.
	}

	if (permitido == 1) {
		pthread_mutex_lock(&mutexFrames);
		while (i < list_size(listaFrames) && acierto == 0) {
			aux = list_get(listaFrames, i);
			if (aux->pid == 0) {
				aux->pid = pid;
				aux->bitModificado = 0;
				aux->bitReferencia = 1;
				idFrame = aux->nroFrame;
				acierto = 1;
			}
			i++;
		}
		pthread_mutex_unlock(&mutexFrames);
		escrituraMemoria(idFrame, 0, size_frames, buffer);
	} else {
		algoritmoDeReemplazo(pid, nroPagina, buffer);
	}

	return 1; //Se logró cargar página en memoria
}

void * solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio,
		uint32_t pid) {

	void * data;
	int nroFrame;
	int pedidoValido;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			data = lecturaMemoria(nroFrame, offset, tamanio);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			return data;
		}
	}
	//TLB Miss
	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) {
		//codigoPagina = pedirleASwapQueMeDeLaPagina(pid, nroPagina);
		//recibirPagina(buffer)
		void * bufferPagina;
		int exito;
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina);
		if (exito == -1) {
			//avisar a Swap para matar proceso. llamo a finalizarPrograma()
			//avisar a Nucleo para matar proceso
			//avisar a CPU para matar proceso
			log_error(logger,
					"No se puede cargar página en memoria del proceso pid %d. No hay frames disponibles.",
					pid);
		}

	} else if(nroFrame == OVERFLOW){
		//avisar que hay overflow
		log_error(logger, "Pedido inválido de pid %d. Fuera del espacio de direcciones.",pid);
	}

	//nroFrame >- 1
	cargarEnTLB(pid, nroPagina, nroFrame);

	data = lecturaMemoria(nroFrame, offset, tamanio);

	printf("Solicitar Bytes \n");

	return data;

}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio,
		void * buffer, uint32_t pid) {

	int nroFrame;

	//chequear si hay stack overflow

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			escrituraMemoria(nroFrame, offset, tamanio, buffer);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			return;
		}
	} //TLB Miss

	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) {
		//codigoPagina = pedirleASwapQueMeDeLaPagina(pid, nroPagina);
		//recibirPagina(buffer)
		void * bufferPagina;
		int exito;
		exito = cargarPaginaEnMemoria(pid, nroPagina, bufferPagina);
		if (exito == -1) {
			//avisar a Swap para matar proceso
			//avisar a Nucleo para matar proceso
			//avisar a CPU para matar proceso
			log_error(logger,
					"No se puede cargar página en memoria del proceso pid %d. No hay frames disponibles.",
					pid);
			pthread_exit(NULL);
		}
	} else if (nroFrame == OVERFLOW){

	}

	//nroFrame > 1
	cargarEnTLB(pid, nroPagina, nroFrame);

	escrituraMemoria(nroFrame, offset, tamanio, buffer);

	printf("Almacenar Bytes \n");
}

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma, int socketNucleo) {

	int largoPrograma = strlen(codigoPrograma) + 1; //?

	int paginasCodigo = largoPrograma / size_frames;
	if ((largoPrograma % size_frames) != 0) {
		paginasCodigo++;
	}

	int respuestaInicializacion;

	pthread_mutex_lock(&mutexSwap);
	enviarPaginasRequeridasASwap(socketSwap, paginasRequeridas);
	respuestaInicializacion = recibirRespuestaInicializacion(socketSwap);
	log_info(logger, "Se envió nuevo programa pid %d a Swap", idPrograma);

	if (respuestaInicializacion == inicioProgramaExito) {
		enviarCodigoASwap(socketSwap, idPrograma, paginasCodigo,
				codigoPrograma);
		enviarRespuestaInicializacionExito(socketNucleo);
	} else {
		enviarRespuestaInicializacionError(socketNucleo);
		log_info(logger,
				"No se pudo inicializar programa pid %d. No hay espacio en Swap",
				idPrograma);
		return;
	}
	pthread_mutex_unlock(&mutexSwap);

	//aca tengo que crear un puntero o una estructura?
	t_nodo_lista_procesos* unNodo = malloc(sizeof(t_nodo_lista_procesos));
	unNodo->pid = idPrograma;
	unNodo->cantPaginas = paginasRequeridas;
	unNodo->framesAsignados = 0;
	unNodo->punteroClock = -1;
	unNodo->lista_paginas = list_create();
	int i;
	for (i = 0; i < paginasRequeridas; i++) {
		t_nodo_lista_paginas* unaPagina = malloc(sizeof(t_nodo_lista_paginas));
		unaPagina->nro_pagina = i;
		unaPagina->status = 'S';
		list_add(unNodo->lista_paginas, unaPagina);
	}

	pthread_mutex_lock(&mutexProcesos);
	list_add(listaProcesos, &unNodo);
	pthread_mutex_unlock(&mutexProcesos);

	log_info(logger, "Se inicializó nuevo programa pid %d", idPrograma);

	free(codigoPrograma);

}

void liberarPaginas(int indiceListaProceso) {
	t_nodo_lista_procesos * procesoAux;
	procesoAux = list_get(listaProcesos, indiceListaProceso);
	list_clean_and_destroy_elements(procesoAux->lista_paginas,
			(void*) destruirPagina);

}

void finalizarPrograma(uint32_t idPrograma) {
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

//TERMINAR FUNCION!!!

//mutex para swap
//InformarSwap() swap borrame tooodo
	log_info(logger, "Se finalizó programa pid %d", idPrograma);

}

void cambioProceso(uint32_t idNuevoPrograma, uint32_t * idProcesoActivo) {

	if (entradasTLB > 0) {
		limpiarEntradasTLB(*idProcesoActivo);
	}

	(*idProcesoActivo) = idNuevoPrograma;
}

void dumpPID(int pid) {
	int indice;
	indice = encontrarPosicionEnListaProcesos((uint32_t) pid);

	if (indice == -1) {
		printf("PID no válido\n");
	}

	t_nodo_lista_procesos* nodoProceso;
	pthread_mutex_lock(&listaProcesos);
	nodoProceso = list_get(listaProcesos, indice);
	pthread_mutex_unlock(&listaProcesos);

	FILE * reporte;
	reporte = fopen("./Dump", "w+");
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
					printf("Ingrese nuevo retardo: \n");

				} while ((scanf("%d%c", &pid, &c) != 2 || c != '\n')
						&& clean_stdin());

				printf("Se ejecutará: Dump del proceso %d\n", pid);

				dumpPID(pid);
			} else if (strncasecmp(comando, "total", 5) == 0) {
				printf("Se ejecutará: Dump de todos los procesos\n");
				//dumpTotal();
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
			log_info(logger, "Se desconectó Núcleo", texto);
			pthread_exit(NULL);

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
			log_error(logger, "Hubo un problema de conexión con Núcleo", texto);
			pthread_exit(NULL);

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
		//char * nroCpu = string_itoa(conexion); para poner nro de cpu

		switch (header) {
		case 0:

			log_info(logger, "Se desconectó CPU nro", texto); //como carajo pongo el nro?
			pthread_exit(NULL);

		case solicitarBytes:
			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size); //deserializacion
			solicitarBytesDeUnaPag(nroPagina, offset, size, idCambioProceso); //operacion
			break;

		case almacenarBytes:
			recibirPedidoAlmacenarBytes(conexion, &nroPagina, &offset, &size);

			bufferPedido = malloc(size);
			recibirBufferPedidoAlmacenarBytes(conexion, size, bufferPedido);
			almacenarBytesEnUnaPag(nroPagina, offset, size, bufferPedido,
					idNuevoProcesoActivo);
			break;
		case cambiarProcesoActivo:
			recibirPID(conexion, &idNuevoProcesoActivo);
			cambioProceso(idNuevoProcesoActivo, &idCambioProceso);
			break;

		default:
			log_error(logger, "Hubo problema de conexion con CPU", texto); //nro cpu?
			pthread_exit(NULL);

			break;
		}
	}

}

