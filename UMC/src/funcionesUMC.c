/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

void cambiarRetardo(int nuevoRetardo) {
	retardo = nuevoRetardo;
}

void destruirProceso(t_nodo_lista_procesos * nodo) {
	free(nodo);
}

void destruirFrame(t_nodo_lista_frames * nodo) {
	free(nodo);
}

int encontrarPosicionEnListaProcesos(int pid) {
	t_nodo_lista_procesos * aux;

	int i = 0;
	int encontrado = 1;

	usleep(retardo * 1000);

	pthread_mutex_lock(&mutexProcesos);
	while ((i < list_size(listaProcesos)) && encontrado != 0) {
		aux = list_get(listaProcesos, i);

		if (aux->pid == pid) {
			encontrado = 0;
		} else {
			i++;
		}
	}
	pthread_mutex_unlock(&mutexProcesos);

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

void reservarFrames(uint32_t pid, int cantPaginas) {
	int i = 0;
	int contador = 0;
	int contadorFrames;
	t_nodo_lista_frames* nodoAux;
	if (cantPaginas < framesPorProceso) {
		contadorFrames = cantPaginas;
		pthread_mutex_lock(&mutexFrames);
		while (i < list_size(listaFrames) && contadorFrames > 0) {
			nodoAux = list_get(listaFrames, i);
			if (nodoAux->pid == 0) {
				contador++;
				contadorFrames--;
				if (contador == 1) {
					inicializarPuntero(pid, i);
				}
			}
		}
		pthread_mutex_unlock(&mutexFrames);
	} else {
		contadorFrames = framesPorProceso;
		pthread_mutex_lock(&mutexFrames);
		while (i < list_size(listaFrames) && contadorFrames > 0) {
			nodoAux = list_get(listaFrames, i);
			if (nodoAux->pid == 0) {
				contador++;
				contadorFrames--;
				if (contador == 1) {
					inicializarPuntero(pid, i);
				}
			}
		}
		pthread_mutex_unlock(&mutexFrames);
	}

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
	int pidEncontrado = 0;
	int aciertoPagina = 0;

	usleep(retardo * 1000);

	pthread_mutex_lock(&mutexProcesos);

	while (i < list_size(listaProcesos) && pidEncontrado == 0) {
		nodoAux = list_get(listaProcesos, i); //list_get empieza en 1 o 0?
		if (nodoAux->pid == pid) {

			while (j < list_size(listaProcesos) && aciertoPagina == 0) {
				nodoPagAux = list_get(nodoAux->lista_paginas, j);

				if (nodoPagAux->nro_pagina == nroPagina) {
					if (nodoPagAux->status == 'M') { //estaEnMemoria
						frame = nodoPagAux->nroFrame;  //aplicarLRU para TLB
						aciertoPagina = 1;
					} else {
						return -1;
						//buscarEnSwap
					}

				}
				j++;
			} //fin While de lista de paginas
		} //fin if nodoAux == pid
		i++;
	}
	pthread_mutex_unlock(&mutexProcesos);

	return (int) frame;
}

void flushTLB() {
	pthread_mutex_lock(&mutexTLB);
	list_clean_and_destroy_elements(TLB, (void *) entradaTLBdestroy);
	pthread_mutex_unlock(&mutexTLB);
}

void limpiarEntradasTLB(uint32_t pid) {
	t_entrada_tlb* nodoAux;
	int i = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB)) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid) {
			nodoAux->pid = 0;
			//list_replace_and_destroy_element(TLB, i, nodoAux,
			//(void*) entradaTLBdestroy);
		}
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
					(void*) destruirFrame); *///no estoy segura del destoy
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
	accesoMemoria ++;
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
	accesoMemoria ++;
	pthread_mutex_unlock(&mutexContadorMemoria);
	actualizarBitReferencia(frame);
	actualizarBitModificado(frame);
	free(buffer);
}

int buscarPuntero(uint32_t pid) {
	int indice;
	int puntero;
	int victima;
	t_nodo_lista_procesos* nodoAux;
	t_nodo_lista_frames* nodoFrame;
	int i;
	indice = encontrarPosicionEnListaProcesos(pid);

	pthread_mutex_lock(&mutexProcesos);
	nodoAux = list_get(listaProcesos, indice);
	pthread_mutex_unlock(&mutexProcesos);
	puntero = nodoAux->punteroClock;

//Primero busco bit de referencia en 0
	pthread_mutex_lock(&mutexFrames);
	while (puntero < list_size(listaFrames)) {
		nodoFrame = list_get(listaFrames, puntero);
		if (nodoFrame->pid == pid) {
			if (nodoFrame->bitReferencia == 0) {
				victima = puntero;
				//corro el puntero
				return victima; //encontre la victima
			}

		}
	}
	pthread_mutex_unlock(&mutexFrames);

}

/*void clock(uint32_t pid, uint32_t paginaNueva, void * codigoPagina){
 //Busco bit de referencia en 0
 buscarPuntero(pid);

 }*/

void actualizarBitUltimoAccesoTLB(uint32_t pid, int nroFrame){
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

void * solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio,
		uint32_t pid) {

	void * data;
	int nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		if (nroFrame > -1) { //TLB Hit
			data = lecturaMemoria(nroFrame, offset, tamanio);
			actualizarBitUltimoAccesoTLB(pid, nroFrame);
			return data;
		}
		//TLB Miss
	}

	nroFrame = buscarEnListaProcesos(pid, nroPagina);

	if (nroFrame == -1) {
		//buscarEnSwap
		//poner el puntero donde la tengo en memprincipal
	}

	data = lecturaMemoria(nroFrame, offset, tamanio);

	printf("Solicitar Bytes \n");

	return data;

	/*si est an memoria, principal, voy a buscar en lista de frames
	 *sino esta, hay que fijarme si puedo asignarle un frame mas al proceso
	 *si le puedo asignar un frame, tengo 2 caminos 1) si tengo frames libres traigo la pagina desde swap
	 *2) sino tengo frames libres, voy a tener aplicar un algoritmo de reemplazo
	 *sino le puedo asignar un framr porque ya supero su cantidad de frames que puede tener en memoria
	 voy a aplicar un algoritmo de reemplazo local*/

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
		//buscarEnSwap
	}

	escrituraMemoria(nroFrame, offset, tamanio, buffer);

	printf("Almacenar Bytes \n");
}

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma) {

	int largoPrograma = strlen(codigoPrograma) + 1; //?
	int paginasCodigo = largoPrograma / size_frames
			+ largoPrograma % size_frames; //not sure

	pthread_mutex_lock(&mutexSwap);
	enviarCodigoASwap(socketSwap, paginasRequeridas, idPrograma, paginasCodigo);
	pthread_mutex_unlock(&mutexSwap);
	log_info(logger, "Se envió nuevo programa a Swap", texto);

//enviarPaginas(enviar pagina x pagina)
	int framesDisponibles = cantidadFramesDisponibles();

	if (framesDisponibles < framesPorProceso
			&& framesDisponibles < paginasRequeridas) {
		//avisar que no se pudo inicializarPrograma a nucleo
		return;
	}

	reservarFrames(idPrograma, paginasRequeridas);

//aca tengo que crear un puntero o una estructura?
	t_nodo_lista_procesos* unNodo = malloc(sizeof(t_nodo_lista_procesos));
	unNodo->pid = idPrograma;
	unNodo->cantPaginas = paginasRequeridas;
//unNodo.framesAsignados = 0;
	unNodo->lista_paginas = list_create();
//unNodo.punteroClock = -1;
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


//enviar rta a nucleo si se pudo inicializar o no

}

void finalizarPrograma(uint32_t idPrograma) {
	int indiceListaProcesos = encontrarPosicionEnListaProcesos(idPrograma);

	pthread_mutex_lock(&mutexProcesos); //NO PONER RETARDO ACA PORQUE YA ESTA EN ENCONTRAR POS EN LISTA PROCESOS
	list_remove_and_destroy_element(listaProcesos, indiceListaProcesos,
			(void*) destruirProceso);
	pthread_mutex_unlock(&mutexProcesos);

	liberarFrames(idPrograma);

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

int clean_stdin() {
	while (getchar() != '\n')
		;
	return 1;
}

void consolaUMC(void) {
	char * comando = malloc(30);
	char c;
	int nuevoRetardo;

	while (1) {
		printf("Ingrese comando:\n");
		scanf("%s", comando);
		if (strncmp(comando, "flush", 5) == 0) {
			printf("Sobre que quiere hacer flush?\n-tlb\n-memory");
			scanf("%s", comando);

			if (strncmp(comando, "tlb", 3) == 0) {
				printf("Se ejecutará: flush TLB\n");
				flushTLB();
				log_info(logger, "Se ejecutó flush TLB");
			} else if (strncmp(comando, "memory", 6) == 0) {
				printf("Se ejecutará: Flush Memoria Principal\n");
				flushMemory();
				log_info(logger, "Se ejecutó flush Memory");
			}
		} else if (strncmp(comando, "dump", 4) == 0) {
			printf("Se ejecutará: Dump\n");
			//dump()
		} else if (strncmp(comando, "retardo", 7) == 0) {
			do {
				printf("Ingrese nuevo retardo: \n");

			} while ((scanf("%d%c", &nuevoRetardo, &c) != 2 || c != '\n')
					&& clean_stdin());

			printf("Se ejecutará: Cambio de retardo\n");
			cambiarRetardo(nuevoRetardo);
			log_info(logger, "Se ejecutó cambio de retardo. El retardo ahora es %d", retardo);
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
			inicializarPrograma(pid, paginas_requeridas, programa);
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

