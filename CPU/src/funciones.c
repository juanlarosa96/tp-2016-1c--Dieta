/*
 * funciones.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */
#include "funciones.h"
#include <stdint.h>
#include <structs.h>

t_posicion_memoria obtenerPosicionPagina(int tamanioPagina, t_pcb unPcb) {
	uint32_t pc = unPcb.pc;

	t_intructions* instrucciones_serializado = unPcb.indice_codigo.instrucciones;
	t_posicion_memoria posicionPagina;
	int tamanio = instrucciones_serializado[pc].offset;
	int numeroPagina = (instrucciones_serializado[pc].start) / tamanioPagina;
	int offset = (instrucciones_serializado[pc].start) % tamanioPagina;

	posicionPagina.offset = offset;
	posicionPagina.pagina = numeroPagina;
	posicionPagina.size = tamanio;
	return posicionPagina;
}

void recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina, char* lineaAnsisop) {
	recibirTodo(socketUMC, lineaAnsisop, posicionPagina.size);
}
int pedirLineaAUMC(int socketUMC, char * lineaAnsisop, t_pcb pcbActual, int tamanioPagina) {
	t_posicion_memoria posicion = obtenerPosicionPagina(tamanioPagina, pcbActual);
	return enviarPedidosDePosicionMemoria(socketUMC, posicion, (void *)lineaAnsisop, tamanioPagina);
}

void recibirBytesDePagina(int socketUMC, int largoPedido, void * buffer) {
	recibirTodo(socketUMC, buffer, largoPedido);
}

int enviarPedidosDePosicionMemoria(int socketUMC, t_posicion_memoria posicion, void * buffer, int tamanioPagina){
	int bytesTotales = posicion.offset + posicion.size;
		int bytesRecibidos = 0, offset = posicion.offset, pagina = posicion.pagina, tamanio = posicion.size;

		if (posicion.size + offset > tamanioPagina) {
			tamanio = tamanioPagina - offset;
		}

		while (bytesTotales >= tamanioPagina) {
			enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
			if(recibirHeader(socketUMC) == pedidoMemoriaFallo) return 1;
			recibirBytesDePagina(socketUMC, tamanio, (void *) buffer + bytesRecibidos);
			bytesTotales -= tamanio;
			bytesRecibidos += tamanio;
			tamanio = tamanioPagina;
			offset = 0;
			pagina++;
		}
		tamanio = bytesTotales - posicion.offset;

		if (tamanio != 0) {
			enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
			if(recibirHeader(socketUMC) == pedidoMemoriaFallo) return 1;
			recibirBytesDePagina(socketUMC, tamanio, (void *) buffer + bytesRecibidos);
		}
		return 0;
}

int enviarAlmacenamientosDePosicionMemoria(int socketUMC, t_posicion_memoria posicion, void * buffer, int tamanioPagina){
	int bytesTotales = posicion.offset + posicion.size;
		int bytesEnviados = 0, offset = posicion.offset, pagina = posicion.pagina, tamanio = posicion.size;

		if (posicion.size + offset > tamanioPagina) {
			tamanio = tamanioPagina - offset;
		}

		while (bytesTotales >= tamanioPagina) {
			enviarPedidoAlmacenarBytes(socketUMC, pagina, offset, tamanio, (char *)buffer);
			if(recibirHeader(socketUMC) == pedidoMemoriaFallo) return 1;
			bytesTotales -= tamanio;
			bytesEnviados += tamanio;
			tamanio = tamanioPagina;
			offset = 0;
			pagina++;
		}
		tamanio = bytesTotales - posicion.offset;

		if (tamanio != 0) {
			enviarPedidoAlmacenarBytes(socketUMC, pagina, offset, tamanio, (char *)buffer);
			if(recibirHeader(socketUMC) == pedidoMemoriaFallo) return 1;
		}
		return 0;
}


void manejadorSIGUSR1(int signal_num){
	if(signal_num == SIGUSR1){

		signalApagado = 1;
	}

}

void avisarANucleoFinalizacionDeCPU(int socketNucleo){
	enviarSenialDeApagadoDeCPU(socketNucleo);


}
