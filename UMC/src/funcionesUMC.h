/*
 * funcionesUMC.h
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESUMC_H_
#define FUNCIONESUMC_H_

#include <stdint.h>
#include <sockets.h>
#include <protocolo.h>

/*-------OperatoriaUMC-------*/

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas, char * codigoPrograma);

void solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio);

void guardarBytesEnUnaPag(int nroPagina, int offset, int tamanio, void * buffer);

void finalizarPrograma(uint32_t idPrograma);

void cambioProceso(uint32_t idPrograma);

/*----Otras----*/

void procesarSolicitudOperacion();


#endif /* FUNCIONESUMC_H_ */
