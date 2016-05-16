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
	uint8_t estado;
}pcbConEstado;

t_list lista_PCB;



enum estados {
	NUEVO = 1, LISTO = 2, EJECUCION = 3, BLOQUEADO = 4, FINALIZADO = 5
};

void manejarCPU(int socketCpu);



#endif /* FUNCIONES_H_ */
