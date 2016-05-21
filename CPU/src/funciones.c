/*
 * funciones.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */
#include "funciones.h"
#include <stdint.h>
#include <Librerias/Librerias/structs.h>

t_posicion_memoria obtenerPosicionPagina(int tamanioPagina, t_pcb unPcb){
	uint32_t pc = unPcb.pc;
	t_metadata_program indice_etiquetas = unPcb.indice_etiquetas;
	t_puntero_instruccion instruccion_inicio = indice_etiquetas.instruccion_inicio;
	t_intructions*	instrucciones_serializado = indice_etiquetas.instrucciones_serializado;
	t_posicion_memoria posicionPagina;
	int tamanio = indice_etiquetas.instrucciones_size;
	int numeroPagina = (instruccion_inicio + instrucciones_serializado[pc].start)/tamanioPagina;
	int offset = (instruccion_inicio + instrucciones_serializado[pc].start)%tamanioPagina;
	if (offset !=0){
		numeroPagina ++;
	}
	posicionPagina.offset = offset;
	posicionPagina.pagina = numeroPagina;
	posicionPagina.size = tamanio;
	return posicionPagina;
}

char* recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina, char* lineaAnsisop){
	//char lineaAnsisop [posicionPagina.size];
	recibirTodo(socketUMC, &lineaAnsisop, posicionPagina.size);
	//return lineaAnsisop;
}

