/*
 * funciones.c
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */
#include "funciones.h"

int iniciarProgramaAnsisop(int cliente, char*archivo,char bitMap[]) {

	int cantPaginasCodigo;
	recibirTodo(cliente, &cantPaginasCodigo, sizeof(int));

	int cantPaginasStack;
	recibirTodo(cliente, &cantPaginasStack, sizeof(int));

	int cantPaginasTotal = cantPaginasCodigo + cantPaginasStack;
	if (chequearMemoriaDisponible(cantPaginasTotal, bitMap) == 0) {
		avisarUMCFallo(cliente);
	} else {
		avisarUMCExito(cliente);
	}

	int pid;
	recibirTodo(cliente, &pid, sizeof(int));

	int i;
	for (i = 0; i < cantPaginasCodigo; i++) {
		char pagina[sizePagina];
		recibirTodo(cliente, pagina, sizePagina);
		archivo[i] = *pagina;
		bitMap[i] = 1;
	}// coregir el tema de que siempre sobreescribe las primeras n paginas del swap

	return 1;// este return es fruta
}



int chequearMemoriaDisponible(int cantPaginas, char bitMap[]) {
	int cantidadContinua, i, cantidadTotal;
	cantidadContinua = i = cantidadTotal = 0;
	int hayContinuas = 0, hayTotales = 0;

	while (i < cantidadDeFrames && !hayContinuas) {
		if (bitMap[i] == 0) {
			cantidadTotal++;
			cantidadContinua++;
		} else {
			cantidadContinua = 0;
		}
		if (cantidadTotal == cantPaginas){hayTotales = 1;}
		if (cantidadContinua == cantPaginas){hayContinuas = 1;}
	}

	if(hayTotales){
			if(hayContinuas){
				return 1;
			} else{
			compactar();
			return 1;
			}
	} else {
	return 0;
	}
}

void avisarUMCFallo(int cliente) {
	int header = inicioProgramaError;
	send(cliente, &header, sizeof(int), 0);
}

void avisarUMCExito(int cliente) {
	int header = inicioProgramaExito;
	send(cliente, &header, sizeof(int), 0);
}

void compactar(){

}
