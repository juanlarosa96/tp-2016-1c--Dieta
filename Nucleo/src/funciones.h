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
#include <commons/collections/queue.h>
#include <sockets.h>
#include <structs.h>
#include <unistd.h>
#include <pthread.h>
#include <protocolo.h>
#include <parser/metadata_program.h>


typedef struct {
	t_pcb pcb;
	int socketConsola;
}t_pcbConConsola;



/*typedef struct t_colaPcb{
	t_pcbConConsola pcb;
	struct t_colaPcb * siguientePcb;
}t_colaPcb;
*/

t_queue *cola_PCBListos;
t_queue *cola_PCBNuevos;
t_queue *cola_PCBFinalizados;
t_queue *cola_PCBBloqueados;

int pidPcb;
int tamanioPagina;
t_list listaConsolas;

void manejarCPU(int socketCpu);

t_pcbConConsola DevolverProcesoColaListos();

t_pcbConConsola sacarPrimeroCola(t_queue * inicioCola);

void AgregarACola(t_pcbConConsola elemento, t_queue * colaFinal);

t_pcb crearPcb(char * programa, int largoPrograma);

int calcularPaginasCodigo (int largoPrograma);

int iniciarUnPrograma(int clienteUMC, t_pcb nuevoPcb, int largoPrograma, char * programa, uint32_t paginasStack);

void AgregarAProcesoColaListos(t_pcbConConsola elemento);

t_pcbConConsola DevolverProcesoColaListos();

#endif /* FUNCIONES_H_ */
