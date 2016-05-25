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


void procesarSolicitudOperacion(){

	while(1){
		//recibirSolicitudes
		//procesarlas
		//send para responderle a CPUs
		//en caso de que llegue un 0 kill el hilo

	}


}

