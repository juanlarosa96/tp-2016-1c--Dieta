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
#include <unistd.h>

int iniciarProgramaAnsisop(int cliente, char*archivo);
int chequearMemoriaDisponible(int cantidadDePaginas,char*archivo);
void avisarUMCFallo(int cliente);
void avisarUMCExito(int cliente);
int compactar(char*archivo);

void guardarPaginas(int cliente, char*archivo);

void enviarPaginas(int cliente,char*archivo);

void finalizarProgramaAnsisop(int cliente);
#endif /* FUNCIONES_H_ */
