/*
 * funcionesUMC.h
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESUMC_H_
#define FUNCIONESUMC_H_

#include <stdint.h>
#include <pthread.h>
#include <sockets.h>
#include <protocolo.h>
#include <unistd.h>
#include <commons/txt.h>
#include "structsUMC.h"
#include "variablesGlobalesUMC.h"

/*-------OperatoriaUMC-------*/

void inicializarPrograma(uint32_t idPrograma, int paginasRequeridas, char * codigoPrograma, int socketNucleo);

void* solicitarBytesDeUnaPag(int nroPagina, int offset, int tamanio, uint32_t pid);

void almacenarBytesEnUnaPag(int nroPagina, int offset, int tamanio, void * buffer, uint32_t pid);

void finalizarPrograma(uint32_t idPrograma);

void cambioProceso(uint32_t idNuevoPrograma, uint32_t * idProcesoActivo);

/*----Hilos hijos del main de UMC----*/

void procesarSolicitudOperacionCPU(int*);
void procesarOperacionesNucleo(void);
void consolaUMC(void);


#endif /* FUNCIONESUMC_H_ */
