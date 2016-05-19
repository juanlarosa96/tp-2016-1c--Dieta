/*
 * funciones.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include "commons/log.h"
#include <commons/collections/list.h>
#include <Librerias/sockets.h>
#include <Librerias/structs.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
	t_pcb pcb;
	int socketConsola;
}t_pcbConConsola;



typedef struct {
	t_pcbConConsola pcb;
	t_pcbConConsola * siguientePcb;
}t_colaPcb;

t_colaPcb cola_PCBListos;
t_colaPcb cola_PCBNuevos;
t_colaPcb cola_PCBFinalizados;
t_colaPcb cola_PCBBloqueados;

void manejarCPU(int socketCpu);

t_pcbConConsola DevolverProcesoColaListos();

t_pcbConConsola sacarPrimeroCola(t_colaPcb * inicioCola);

void AgregarACola(t_pcbConConsola elemento, t_colaPcb cola);


#endif /* FUNCIONES_H_ */
