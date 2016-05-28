/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

void destruirProceso(t_nodo_lista_procesos * nodo){
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

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas,
		char * codigoPrograma) {
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

	//fijarse si hay que poner un mutex aca
	list_add(listaProcesos, &unNodo);

	//enviarCodigoASwap()
	//loggear envio de codigo a swap
	//loggear que se pudo crear

}

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio) {

	/*primero busco en mi lista de procesos
	 *de ahi accedo a la lista de paginas
	 *ahi me fijo si esta en memoria principal
	 *si est an memoria, principal, voy a buscar en lista de frames
	 *sino esta, hay que fijarme si puedo asignarle un frame mas al proceso
	 *si le puedo asignar un frame, tengo 2 caminos 1) si tengo frames libres traigo la pagina desde swap
	 *2) sino tengo frames libres, voy a tener aplicar un algoritmo de reemplazo
	 *sino le puedo asignar un framr porque ya supero su cantidad de frames que puede tener en memoria
	 voy a aplicar un algoritmo de reemplazo local*/

	printf("Solicitar Bytes \n");
}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio,
		void * buffer) {

	printf("Almacenar Bytes \n");
}

void finalizarPrograma(uint32_t idPrograma) {
	int indiceListaProcesos = encontrarPosicionEnListaProcesos(idPrograma);

	//mutex?
	list_remove_and_destroy_element(listaProcesos, indiceListaProcesos,(void*) destruirProceso);
	//TERMINAR FUNCION!!!

	//informar al swap

}

void cambioProceso(uint32_t idPrograma) {

}

/*void procesarSolicitudOperacionNucleo(t_datos_hilo datosSockets){

 while(1) {

 int header = recibirHeader(datosSockets -> socketCliente);


 switch(header){


 case 8: //solicitarBytes
 uint32_t pid;
 int * largoPrograma;
 uint32_t paginas_codigo;
 char * programa;
 recibirInicializacionPrograma(datosSockets -> listener, &pid, &largoPrograma, programa, &paginas_codigo);

 //blabla bla recibir programa
 //recibir lo demas
 solicitarBytesDeUnaPag();
 break;
 case 9: //almacenarBytes
 //blabla procesar bytes
 //recibir lo demas
 guardarBytesDeUnaPag();
 break;

 default:




 break;
 //vete de aqui hilo
 //eclipse haceme el tp

 }


 }
 }
 */

void procesarSolicitudOperacionCPU(int conexion) {

	//cola de pedidos?

	while (1) {
		int header = recibirHeader(conexion);

		uint32_t nroPagina;
		uint32_t offset;
		uint32_t size;
		int lenBufferPedido;
		char * bufferPedido;


		switch (header) {
		case 0:

			//se desconectó cpu

			pthread_exit(NULL);

		case 8: //solicitarBytes //ver el tema de la constante
			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size);//deserializacion
			solicitarBytesDeUnaPag(nroPagina, offset, size); //operacion
			break;

		case 9: //almacenarBytes
			recibirPedidoAlmacenarBytes(conexion, &nroPagina, &offset, &size,
					&lenBufferPedido);

			bufferPedido = malloc(lenBufferPedido);
			recibirBufferPedidoAlmacenarBytes(conexion, lenBufferPedido,
					bufferPedido);

			free(bufferPedido);

			break;

		default:
			// hubo problema de conexión //loggearlo
			//chau hilo
			pthread_exit(NULL);

			break;
		}
	}

}



