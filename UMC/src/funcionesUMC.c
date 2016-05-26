/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas, char * codigoPrograma){

}

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio){

};

void guardarBytesEnUnaPag(int nroPagina, int offset, int tamanio, void * buffer){

};

void finalizarPrograma(uint32_t idPrograma){

};

void cambioProceso(uint32_t idPrograma){

};


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

		switch(header){
			case 8: //solicitarBytes //ver el tema de la constante

			recibirSolicitudDeBytes(conexion, &nroPagina, &offset, &size);	//deserializacion
			solicitarBytesDeUnaPag(nroPagina, offset, size); //operacion
			break;
			case 9: //almacenarBytes

			break;
			default:
			//se desconectó CPU o hubo problema de conexión
			//chau hilo

			break;
		}
	}


}

