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
	primitivaImprimir = 12,
	finalizacionPrograma = 13,
	cambiarProcesoActivo = 14,
	headerEntradaSalida = 15,
	pedidoMemoriaOK = 16,
	pedidoMemoriaFallo = 17,
	abortarPrograma = 18,
	guardarPaginasEnSwap = 19,
	pedidoPaginaASwap = 20,
	inicializarProgramaSwap = 21,
	headerWait = 22,
	headerSignal = 23,
	finDeQuantum = 24,
	quantumSleep = 25,
	quantumUnidades = 26,
	pedidoVariableCompartida = 27,
	asignacionVariableCompartida = 28,
	finalizacionCPU = 29,
	headerBloquear = 30,
	headerSeguir = 31,
	CPUListo = 32
};
int recibirHeader(int socketOrigen);
void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo);
void recibirProgramaAnsisop(int socketOrigen, char * codigo, int largoCodigo);
int recibirLargoProgramaAnsisop(int socketOrigen);
//header: 3
int recibirTamanioPagina(int socketOrigen);
void enviarTamanioPagina(int socketDestino, int tamanioPagina);
void enviarPedidoPaginas(int socketUMC, int cantidadPaginas);
void enviarInicializacionPrograma(int socketUMC,uint32_t pid,int largoPrograma,char * programa, uint32_t paginas_codigo);
void recibirInicializacionPrograma(int socketUMC, uint32_t *pid, uint32_t *paginasRequeridas, int *largoCodigo);
void enviarRespuestaInicializacionExito(int socketDestino);
void enviarRespuestaInicializacionError(int socketDestino);
void recibirCodigoInicializarPrograma(int socketUMC, int largoCodigo, char *codigo);
int recibirRespuestaInicializacion(int socketUMC);
//header: 8
void enviarSolicitudDeBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size);
void recibirSolicitudDeBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size);
int recibirRespuestaCPU(int socketCpu, int * respuesta);
void recibirResultadoDeEjecucionAnsisop(int socketNucleo,char ** mensaje, int *largoMensaje);
int recibirLargoResultadoDeEjecucionAnsisop(int socketNucleo);
void enviarResultadoDeEjecucionAnsisop(int socketDestino, char * mensaje, int largoMensaje);
//header: 9
void enviarPedidoAlmacenarBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size, void* buffer);
void recibirPedidoAlmacenarBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size);
void recibirBufferPedidoAlmacenarBytes(int socketUMC, int largoPedido, char * buffer);
void enviarValorAImprimir(int socketNucleo, uint32_t id_proceso, char * texto);
void recibirValorAImprimir(int socketOrigen, uint32_t *id_proceso, int *largoTexto, char ** texto);
void enviarPcb(int socketCPU, t_pcb pcb);
t_pcb recibirPcb (int socketNucleo);

//header: 13 //header: 14 (las operaciones dentro de UMC son diferentes, pero se manejan los mismos datos)
void enviarFinalizacionProgramaUMC(int socketUMC, uint32_t pid);
void enviarCambioProcesoActivo(int socketUMC, uint32_t pid);
void recibirPID(int socketUMC, uint32_t * pid);
void enviarFinalizacionProgramaConsola(int socketConsola);

//header: 15
void recibirEntradaSalida(int socketOrigen, int *largoNombreDispositivo, char ** nombreDispositivo, int *tiempo);
void enviarEntradaSalida(int socketNucleo, t_pcb pcb, t_nombre_dispositivo dispositivo, int tiempo);

//header: 21
void enviarPaginasRequeridasASwap(int, int);
void enviarCodigoASwap(int socketSwap, int cantPaginas, uint32_t pid, int tamanioCodigo);

//header: 22 // header 23
void enviarWait(int socketNucleo, int id_proceso, t_nombre_semaforo nombreSemaforo);
void recibirWait(int socketOrigen, uint32_t *id_proceso,int *largoNombreSemaforo, t_nombre_semaforo * nombreSemaforo);
void enviarSignal(int socketNucleo, int id_proceso, t_nombre_semaforo nombreSemaforo);
void recibirSignal(int socketOrigen, uint32_t *id_proceso,int *largoNombreSemaforo, t_nombre_semaforo * nombreSemaforo);

int recibirCantidadQuantum(int socketOrigen);
void enviarUnidadesQuantum(int socketCPU, int  unidades);
void enviarSleepQuantum(int socketCPU, int  sleep);
void enviarFinalizacionProgramaNucleo(int socketNucleo);
void enviarAbortarProgramaNucleo(int socketNucleo);
int pedirCompartidaNucleo(int socketNucleo, char * variable, int * punteroVariable);
void asignarCompartidaNucleo(int socketNucleo, char * variable, int valor);
void pedirPaginaASwap(int socketSwap, uint32_t pid, int nroPagina); //check tipo ultimo parametro
void enviarAbortarProceso(int socketCPU);
void enviarPedidoMemoriaOK(int socketCPU);

//header: 29
void enviarSenialDeApagadoDeCPU(int socketNucleo);
void avisarANucleoCPUListo(int socketNucleo);

void enviarRespuestaSemaforo(int socketCpu, int respuesta);

void recibirVariableCompartidaConValor(int socketCPU, char ** nombre, int * valor);
void recibirVariableCompartida(int socketCPU, char ** nombre);
void enviarValorVariableCompartida(int socketCpu, int valor);

#endif /* LIBRERIASSOENWORSKPACE_PROTOCOLO_H_ */
