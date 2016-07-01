/*
 * funciones.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_


#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <parser/parser.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/config.h>
#include <structs.h>
#include <sockets.h>
#include <protocolo.h>
#include <signal.h>
#include "variables_globales.h"
#include <pthread.h>



t_posicion_memoria obtenerPosicionPagina(int tamanioPagina, t_pcb unPcb);
void recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina, char* holis); //no se quien fue pero mejor nombre de variable de la historia
int pedirLineaAUMC(int socketUMC,char * lineaAnsisop, t_pcb pcbActual, int tamanioPagina);
int recibirBytesDePagina(int socketUMC, int largoPedido, void * buffer);
int enviarPedidosDePosicionMemoria(int socketUMC, t_posicion_memoria posicion, void * buffer, int tamanioPagina);
int enviarAlmacenamientosDePosicionMemoria(int socketUMC, t_posicion_memoria posicion, void * buffer, int tamanioPagina);
void manejadorSIGUSR1(int signal_num);
void avisarANucleoFinalizacionDeCPU(int socketNucleo);
void hiloSignalYHeader();
void borrarBarraTesYEnesDeString(char* variable);
void destruirPcb(t_pcb pcb);
void destruirRegistroStack(t_registro_pila * registro);


#endif /* FUNCIONES_H_ */
