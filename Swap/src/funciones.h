/*
 * funciones.h
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <stdio.h>
#include <string.h>
#include "VariablesGlobales.h"
#include <sockets.h>
#include <protocolo.h>
#include "Structs.h"

int iniciarProgramaAnsisop(int cliente, char*archivo,char bitMap[]);
int chequearMemoriaDisponible(int cantidadDePaginas,char bitMap[]);
void avisarUMCFallo(int cliente);
void avisarUMCExito(int cliente);
void compactar();

#endif /* FUNCIONES_H_ */
