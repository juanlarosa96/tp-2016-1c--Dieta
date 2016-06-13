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
#include "hexdump.h"

/*----Hilos hijos del main de UMC----*/

void procesarSolicitudOperacionCPU(int*);
void procesarOperacionesNucleo(int*);
void consolaUMC(void);


#endif /* FUNCIONESUMC_H_ */
