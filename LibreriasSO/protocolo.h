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

#include "pila.h"

enum headers { //Constantes que identifican los headers de los mensajes

	programaAnsisop = 1,
	largoProgramaAnsisop = 2,
	tamanioDePagina = 3,
	headerPcb = 4,
	iniciarPrograma = 11,
	inicioProgramaExito = 6,
	inicioProgramaError = 7,
	solicitarBytes = 8,
	almacenarBytes = 9,
	respuestaCPU = 10,
	resultadoEjecucion = 5,
	primitivaImprimir = 12

};
int recibirHeader(int socketOrigen);
void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo);
void recibirProgramaAnsisop(int socketOrigen, char * codigo, int largoCodigo);
int recibirLargoProgramaAnsisop(int socketOrigen);
//header: 3
int recibirTamanioPagina(int socketOrigen);
void enviarTamanioPagina(int socketDestino, int tamanioPagina);
t_pcb recibirPcb(int socketOrigen);
void enviarPedidoPaginas(int socketUMC, int cantidadPaginas);
void enviarInicializacionPrograma(int socketUMC,uint32_t pid,int largoPrograma,char * programa, uint32_t paginas_codigo);
void recibirInicializacionPrograma(int socketUMC,uint32_t *pid,int* largoPrograma,char * programa, uint32_t *paginas_codigo);
int recibirRespuestaInicializacion(int socketUMC);
//header: 8
void enviarSolicitudDeBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size);
void recibirSolicitudDeBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size);
int recibirRespuestaCPU(int socketCpu, int * respuesta);
void recibirResultadoDeEjecucionAnsisop(int socketNucleo,char * mensaje, int largoMensaje);
int recibirLargoResultadoDeEjecucionAnsisop(int socketNucleo);
void enviarResultadoDeEjecucionAnsisop(int socketDestino, char * mensaje, int largoMensaje);
//header: 9
void enviarPedidoAlmacenarBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size, int largoBuffer, char * buffer);
void recibirPedidoAlmacenarBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size, int * largoBuffer);
void recibirBufferPedidoAlmacenarBytes(int socketUMC, int largoPedido, char * buffer);
void enviarValorAImprimir(int socketNucleo, uint32_t id_proceso, char * texto);
void recibirValorAImprimir(int socketOrigen, uint32_t *id_proceso, int *largoTexto, char * texto);
void enviarPcb(int socketCPU, t_pcb pcb);


#endif /* LIBRERIASSOENWORSKPACE_PROTOCOLO_H_ */
