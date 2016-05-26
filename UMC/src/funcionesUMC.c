/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas, char * codigoPrograma){

	printf("\n Inicializar Programa \n");
}

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio){

	printf("Solicitar Bytes \n");
}

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio, void * buffer){

	printf("Almacenar Bytes \n");
}

void finalizarPrograma(uint32_t idPrograma){

	printf("FinalizarPrograma \n");
}

void cambioProceso(uint32_t idPrograma){

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



void procesarSolicitudOperacionCPU(int conexion){

	//cola de pedidos?

	while(1) {

		int header = recibirHeader(conexion);

		uint32_t nroPagina;
		uint32_t offset;
		uint32_t size;
		int lenBufferPedido;
		char * bufferPedido;

		switch(header){
			case 0:
			//se desconectó cpu

			pthread_exit(NULL);

			case 8: //solicitarBytes //ver el tema de la constante
			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size);	//deserializacion
			solicitarBytesDeUnaPag(nroPagina, offset, size); //operacion
			break;

			case 9: //almacenarBytes
			recibirPedidoAlmacenarBytes(conexion, &nroPagina, &offset, &size, &lenBufferPedido);

			bufferPedido = malloc(lenBufferPedido);
			recibirBufferPedidoAlmacenarBytes(conexion,lenBufferPedido, bufferPedido);

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

