/*
 * funcionesUMC.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "funcionesUMC.h"
#include <stdint.h>

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

void procesarSolicitudOperacionCPU(t_datos_hilo datosSockets){

	while(1) {

			int header = recibirHeader(datosSockets -> socketCliente);


			/*switch(header){


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

			}*/


		}
}



void procesarSolicitudOperacionCPU(t_datos_hilo datosSockets){

	while(1) {

		int header = recibirHeader(datosSockets -> socketCliente);


		switch(header){

			case 8: //solicitarBytes
			//recibir lo demas
			solicitarBytesDeUnaPag();
			break;
			case 9: //almacenarBytes
			//blabla procesar bytes
			//recibir lo demas
			guardarBytesDeUnaPag();
			break;

			default:
			//se desconectó CPU o hubo problema de conexión
			//chau hilo

			break;


		}
		//recibirSolicitudes
		//procesarlas
		//send para responderle a CPUs

	}


}

