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
char* texto;
pthread_mutex_t mutexFrames;
pthread_mutex_t mutexProcesos;
t_log* logger;

#endif /* VARIABLESGLOBALESUMC_H_ */
