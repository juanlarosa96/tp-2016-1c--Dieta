/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

#define TAMBUFFER 100

/*char *borrarEspaciosInnecesarios(char *string){
 char **cadenaSinEspacios = malloc(TAMBUFFER);
 *cadenaSinEspacios = string;
 string_trim(cadenaSinEspacios);

 return (*cadenaSinEspacios);
 }*/

/*void consolaUMC(void) {
 char * buffer;
 size_t bufferSize = TAMBUFFER;
 buffer = malloc(bufferSize * sizeof(char));


 if(buffer == NULL){
 log_error(logger, "No se pudo reservar memoria para recibir comandos por consola de UMC");
 pthread_exit(NULL);
 }


 while(1){
 printf("Ingresar comando:\n");
 getline(&buffer,&bufferSize,stdin);
 //  printf("%zu characters were read.\n",characters);

 //Validaciones ?
 string_trim(*buffer); //borro espacios innecesarios del buffer //ver funcion definida mas arriba

 //terminar funcion

 }

 }*/

void destruirProceso(t_nodo_lista_procesos * nodo) {
	free(nodo);
}

int encontrarPosicionEnListaProcesos(int pid) {
	t_nodo_lista_procesos * aux;

	int i = 0;
	int encontrado = 1;

	//fijarse si hay que poner mutex
	while ((i < list_size(listaProcesos)) && encontrado != 0) {
		aux = list_get(listaProcesos, i);

		if (aux->pid == pid) {
			encontrado = 0;
		} else {
			i++;
		}
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
			list_replace(listaFrames, i, &nodoAux);
		}
		i++;
	}
	pthread_mutex_unlock(&mutexFrames);
}

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma) {

	int largoPrograma = strlen(codigoPrograma) + 1;
	int paginasCodigo = largoPrograma/size_frames + largoPrograma%size_frames; //not sure

	pthread_mutex_lock(&mutexSwap);
	enviarCodigoASwap(socketSwap, paginasRequeridas, idPrograma, paginasCodigo);
	pthread_mutex_unlock(&mutexSwap);
	log_info(logger, "Se envió nuevo programa a Swap", texto);

	//enviarPaginas(enviar pagina x pagina)

	//aca tengo que crear un puntero o una estructura?
	t_nodo_lista_procesos unNodo;
	unNodo.pid = idPrograma;
	unNodo.cantPaginas = paginasRequeridas;
	unNodo.lista_paginas = list_create();
	int i;
	for (i = 0; i < paginasRequeridas; i++) {
		t_nodo_lista_paginas unaPagina;
		unaPagina.nro_pagina = i;
		unaPagina.status = 'S';
		list_add(unNodo.lista_paginas, &unaPagina); //por referencia?
	}

	pthread_mutex_lock(&mutexProcesos);
	list_add(listaProcesos, &unNodo);
	pthread_mutex_unlock(&mutexProcesos);


}

uint32_t buscarEnTLB(uint32_t pid, int nroPagina) {
	uint32_t frame;

	t_entrada_tlb* nodoAux;
	int i = 0;
	int acierto = 0;

	pthread_mutex_lock(&mutexTLB);

	while (i < list_size(TLB) && acierto == 0) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid && nodoAux->nroPagina == (uint32_t) nroPagina) {
			frame = nodoAux->nroFrame;
			acierto = 1;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexTLB);

	return frame;
}

void * lecturaMemoria(uint32_t frame, uint32_t offset, uint32_t tamanio) {
	void * bytes = malloc(tamanio);
	int posicion = (int) frame * size_frames + offset;
	void * posicionAux = memoriaPrincipal + posicion;
	memcpy(bytes, posicionAux, tamanio);
	return bytes;
}

void * solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio,
		uint32_t pid) {

	void * data;
	uint32_t nroFrame;

	if (entradasTLB > 0) {
		nroFrame = buscarEnTLB(pid, nroPagina);
		data = lecturaMemoria(nroFrame, offset, tamanio);
	}

	/*si est an memoria, principal, voy a buscar en lista de frames
	 *sino esta, hay que fijarme si puedo asignarle un frame mas al proceso
	 *si le puedo asignar un frame, tengo 2 caminos 1) si tengo frames libres traigo la pagina desde swap
	 *2) sino tengo frames libres, voy a tener aplicar un algoritmo de reemplazo
	 *sino le puedo asignar un framr porque ya supero su cantidad de frames que puede tener en memoria
	 voy a aplicar un algoritmo de reemplazo local*/

	printf("Solicitar Bytes \n");

	return data;

}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio,
		void * buffer) {

	printf("Almacenar Bytes \n");
}

void limpiarEntradasTLB(uint32_t pid) {
	t_entrada_tlb* nodoAux;
	int i = 0;

	pthread_mutex_lock(&mutexTLB);
	while (i < list_size(TLB)) {
		nodoAux = list_get(TLB, i);
		if (nodoAux->pid == pid) {
			nodoAux->pid = 0;
			list_replace(TLB, i, &nodoAux);
		}
	}
	pthread_mutex_unlock(&mutexTLB);

}

void finalizarPrograma(uint32_t idPrograma) {
	int indiceListaProcesos = encontrarPosicionEnListaProcesos(idPrograma);

	pthread_mutex_lock(&mutexProcesos);
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

}

void cambioProceso(uint32_t idNuevoPrograma, uint32_t * idProcesoActivo) {

	if (entradasTLB > 0) {
		limpiarEntradasTLB(*idProcesoActivo);
	}

	(*idProcesoActivo) = idNuevoPrograma;
}

void procesarOperacionesNucleo(int socketNucleo) {

	while (1) {

		int header = recibirHeader(socketNucleo);
		uint32_t pid;
		int largoPrograma;
		uint32_t paginas_codigo;
		char * programa;

		switch (header) {

		case 0:
			log_info(logger, "Se desconectó Núcleo", texto);
			pthread_exit(NULL);

		case 11: //Inicializacion Programa

			recibirInicializacionPrograma(socketNucleo, &pid, &largoPrograma,
					programa, &paginas_codigo);
			//blabla bla recibir programa
			//recibir lo demas

			break;

		default:
			log_error(logger, "Hubo un problema de conexión con Núcleo", texto);
			pthread_exit(NULL);

		}

	}
}

void procesarSolicitudOperacionCPU(int conexion) {

	uint32_t idCambioProceso;

	while (1) {
		int header = recibirHeader(conexion);

		uint32_t nroPagina;
		uint32_t offset;
		uint32_t size;
		int lenBufferPedido;
		char * bufferPedido;
		uint32_t idNuevoProcesoActivo;
		//char * nroCpu = string_itoa(conexion); para poner nro de cpu

		switch (header) {
		case 0:

			log_info(logger, "Se desconectó CPU nro", texto);//como carajo pongo el nro?
			pthread_exit(NULL);

		case 8: //solicitarBytes //ver el tema de la constante, no me las reconoce
			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size); //deserializacion
			solicitarBytesDeUnaPag(nroPagina, offset, size, idCambioProceso); //operacion
			break;

		case 9: //almacenarBytes
			recibirPedidoAlmacenarBytes(conexion, &nroPagina, &offset, &size,
					&lenBufferPedido);

			bufferPedido = malloc(lenBufferPedido);
			recibirBufferPedidoAlmacenarBytes(conexion, lenBufferPedido,
					bufferPedido);

			free(bufferPedido);

			break;
		case 14: //cambiarProcesoActivo

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

