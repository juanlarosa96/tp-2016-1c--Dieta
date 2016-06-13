/*
 * VariablesGlobales.h
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */
#include <commons/collections/list.h>
#include <commons/log.h>
#ifndef VARIABLESGLOBALES_H_
#define VARIABLESGLOBALES_H_

int cantidadDeFrames;
int sizePagina;
int retardoCompactacion;
int retardoAcceso;
int *bitMap;
t_list *listaProcesos;
t_log* logger;

#endif /* VARIABLESGLOBALES_H_ */
