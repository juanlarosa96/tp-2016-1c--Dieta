/*
 * variablesGlobalesUMC.h
 *
 *  Created on: 27/5/2016
 *      Author: utnso
 */

#ifndef VARIABLESGLOBALESUMC_H_
#define VARIABLESGLOBALESUMC_H_

#include <commons/log.h>

t_list * listaFrames;
t_list * listaProcesos;
t_list * TLB;
char* texto; //?

pthread_mutex_t mutexFrames;
pthread_mutex_t mutexProcesos;
pthread_mutex_t mutexTLB;
pthread_mutex_t mutexSwap;
pthread_mutex_t mutexAccesoMem;


uint16_t entradasTLB;
uint32_t accesoMemoria;

t_log* logger;

void * memoriaPrincipal;

int size_frames;
int retardo;
int socketSwap;



#endif /* VARIABLESGLOBALESUMC_H_ */
