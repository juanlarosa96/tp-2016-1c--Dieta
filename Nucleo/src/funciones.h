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
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>

t_log* logger;
char *texto;

pthread_mutex_t mutexColaListos;
pthread_mutex_t mutexColaFinalizados;
pthread_mutex_t mutexListaConsolas;
pthread_mutex_t mutexListaFinalizacionesPendientes;

sem_t semaforoColaListos;



typedef struct {
	int pid;
	int socketConsola;
}t_pidConConsola;

typedef struct {
	t_pcb pcb;
	int socketConsola;
}t_pcbConConsola;

typedef struct {
	int retardoDispositivo;
	pthread_mutex_t * mutex;
	sem_t * semaforo;
	t_queue * colaBloqueados;
}t_parametroThreadDispositivoIO;

typedef struct {
	t_pcbConConsola pcb;
	int unidadesTiempoIO;
}t_pcbBloqueado;

/*typedef struct t_colaPcb{
	t_pcbConConsola pcb;
	struct t_colaPcb * siguientePcb;
}t_colaPcb;
*/

t_queue *cola_PCBListos;
t_queue *cola_PCBNuevos;
t_queue *cola_PCBFinalizados;
t_queue *cola_PCBBloqueados;

int clienteUMC;
int pidPcb;
int tamanioPagina;
char ** vectorDispositivos;
char ** vectorRetardoDispositivos;
pthread_mutex_t ** vectorMutexDispositivosIO;
sem_t * vectorSemaforosDispositivosIO;
t_queue ** vectorColasBloqueados;
int cantidadQuantum;
int retardoQuantum;


t_list *listaConsolas;
t_list *listaFinalizacionesPendientes;

void manejarCPU(void * socket);

t_pcbConConsola DevolverProcesoColaListos();

t_pcbConConsola sacarPrimeroCola(t_queue * inicioCola);

void AgregarACola(t_pcbConConsola elemento, t_queue * colaFinal);

t_pcb crearPcb(char * programa, int largoPrograma);

int calcularPaginasCodigo (int largoPrograma);

int iniciarUnPrograma(int clienteUMC, t_pcb nuevoPcb, int largoPrograma, char * programa, uint32_t paginasStack);

void AgregarAProcesoColaListos(t_pcbConConsola elemento);

t_pcbConConsola DevolverProcesoColaListos();

void finalizarProceso(t_pcbConConsola siguientePcb);
void abortarProceso(t_pcbConConsola siguientePcb);
void AgregarAProcesoColaFinalizados(t_pcbConConsola elemento);
void crearHilosEntradaSalida();
void manejarIO(t_parametroThreadDispositivoIO * datosHilo);
void ponerEnColaBloqueados(t_pcbConConsola siguientePcb, char * nombre, int largo, int tiempo);
void destruirPcb(t_pcb pcb);
void destruirRegistroStack(t_registro_pila * registro);
#endif /* FUNCIONES_H_ */
