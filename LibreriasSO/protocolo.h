/*
 * protocolo.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef LIBRERIASSO_PROTOCOLO_H_
#define LIBRERIASSO_PROTOCOLO_H_

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "structs.h"

enum headers { //Constantes que identifican los headers de los mensajes

	programaAnsisop = 1,
	largoProgramaAnsisop = 2,
	tamanioDePagina = 3,
	headerPcb = 4,
	resultadoEjecucion = 5
};
int recibirHeader(int socketOrigen);
void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo);
void recibirProgramaAnsisop(int socketOrigen, char * codigo, int largoCodigo);
int recibirLargoProgramaAnsisop(int socketOrigen);
int recibirTamanioPagina(int socketOrigen);
void enviarTamanioPagina(int socketDestino, int tamanioPagina);
t_pcb recibirPcb(int socketOrigen);
void enviarPedidoPaginas(int socketUMC, int cantidadPaginas);
void recibirResultadoDeEjecucionAnsisop(int socketNucleo,char * mensaje, int largoMensaje);
int recibirLargoResultadoDeEjecucionAnsisop(int socketNucleo);
void enviarResultadoDeEjecucionAnsisop(int socketDestino, char * mensaje, int largoMensaje);

#endif /* LIBRERIASSOENWORSKPACE_PROTOCOLO_H_ */
