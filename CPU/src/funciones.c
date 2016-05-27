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
	t_metadata_program indice_etiquetas = unPcb.indice_etiquetas;
	t_puntero_instruccion instruccion_inicio =
			indice_etiquetas.instruccion_inicio;
	t_intructions* instrucciones_serializado =
			indice_etiquetas.instrucciones_serializado;
	t_posicion_memoria posicionPagina;
	int tamanio = instrucciones_serializado[pc].offset;
	int numeroPagina =
			(instruccion_inicio + instrucciones_serializado[pc].start)
					/ tamanioPagina;
	int offset = (instruccion_inicio + instrucciones_serializado[pc].start)
			% tamanioPagina;
	if (offset != 0) {
		numeroPagina++;
	}
	posicionPagina.offset = offset;
	posicionPagina.pagina = numeroPagina;
	posicionPagina.size = tamanio;
	return posicionPagina;
}

char* recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina,
		char* lineaAnsisop) {
	recibirTodo(socketUMC, lineaAnsisop, posicionPagina.size);
}
void pedirLineaAUMC(int socketUMC, char * lineaAnsisop, t_pcb pcbActual,
		int tamanioPagina) {
	t_posicion_memoria posicion = obtenerPosicionPagina(tamanioPagina,
			pcbActual);
	int bytesTotales = posicion.offset + posicion.size;
	int bytesRecibidos = 0, offset = posicion.offset, pagina = posicion.pagina,
			tamanio = posicion.size;

	if (posicion.size + offset > tamanioPagina) {
		tamanio = tamanioPagina - offset;
	}

	while (bytesTotales >= tamanioPagina) {
		enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
		//recibirBytesDeUmc(socketUmc,lineaAnsisop + bytesRecibidos,tamanio);
		bytesTotales -= tamanio;
		bytesRecibidos += tamanio;
		tamanio = tamanioPagina;
		offset = 0;
		pagina++;
	}
	tamanio = bytesTotales;
	enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
	//recibirBytesDeUmc(socketUmc,lineaAnsisop + bytesRecibidos,tamanio);
}

