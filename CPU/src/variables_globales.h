/*
 * variables_globales.h
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */

#ifndef VARIABLES_GLOBALES_H_
#define VARIABLES_GLOBALES_H_
#define TAM_VAR 4

#include <structs.h>
#include <semaphore.h>

int socketNucleo;
int socketUMC;
int tamanioPagina;
int huboEntradaSalida;
int huboSaltoLinea;

t_pcb pcbRecibido;

int sigoEjecutando;
int signalApagado;

t_log * logger;

sem_t semComenzarQuantum;
sem_t semRecibirHeader;
int header;

#endif /* VARIABLES_GLOBALES_H_ */
