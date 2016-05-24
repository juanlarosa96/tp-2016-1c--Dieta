/*
 * protocolo.h
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

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
	iniciarPrograma = 5,
	inicioProgramaExito = 6,
	inicioProgramaError = 7
};
int recibirHeader(int socketOrigen);
void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo);
void recibirProgramaAnsisop(int socketOrigen, char * codigo, int largoCodigo);
int recibirLargoProgramaAnsisop(int socketOrigen);
int recibirTamanioPagina(int socketOrigen);
void enviarTamanioPagina(int socketDestino, int tamanioPagina);
t_pcb recibirPcb(int socketOrigen);
void enviarPedidoPaginas(int socketUMC, int cantidadPaginas);
void enviarInicializacionPrograma(int socketUMC,uint32_t pid,int largoPrograma,char * programa, uint32_t paginas_codigo);
void recibirInicializacionPrograma(int socketUMC,uint32_t *pid,int* largoPrograma,char * programa, uint32_t *paginas_codigo);
int recibirRespuestaInicialicacion(int socketUMC);
#endif /* PROTOCOLO_H_ */
