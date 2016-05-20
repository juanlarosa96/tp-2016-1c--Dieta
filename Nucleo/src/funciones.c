/*
 * funciones.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#include "funciones.h"

void manejarCPU(int socketCpu){

	/*
	 * chequeo que el CPU no haya hecho un hot plug
	 * t_pcb unPcb = obtenerPcbEnEstadoDeListo();
	 * Se ejecuta un quantum
	 * Se pone el pcb en estado de ejecucion
	 * Se envia pcb al cpu
	 * termina cuantum/interrupcion
	 * Se pone pcb en cola de listo/bloqueado segun corresponda
	 * repetir
	 */
}


void AgregarACola(t_pcbConConsola elemento, t_colaPcb * colaFinal){
	t_colaPcb * nuevoElementoCola = malloc(sizeof(t_colaPcb));
	nuevoElementoCola->pcb = elemento;
	colaFinal->siguientePcb = nuevoElementoCola;
	nuevoElementoCola->siguientePcb = (void *) 0;
	colaFinal = nuevoElementoCola;
	return;
}

t_pcbConConsola sacarPrimeroCola(t_colaPcb * inicioCola){
	t_pcbConConsola elemento;
	t_colaPcb * auxiliar;
	elemento = inicioCola->pcb;
	auxiliar = inicioCola;
	inicioCola = inicioCola->siguientePcb;
	free(auxiliar);
	return elemento;

}


t_pcbConConsola DevolverProcesoColaListos(){
	return (sacarPrimeroCola(&cola_PCBListos));
}

t_pcbConConsola DevolverProcesoColaNuevos(){
	return (sacarPrimeroCola(&cola_PCBNuevos));
}

t_pcbConConsola DevolverProcesoColaFinalizados(){
	return (sacarPrimeroCola(&cola_PCBFinalizados));
}

t_pcbConConsola DevolverProcesoColaLBloqueados(){
	return (sacarPrimeroCola(&cola_PCBListos));
}

void AgregarAProcesoColaListos(t_pcbConConsola elemento){
	AgregarACola(elemento,&cola_PCBListos);
}

void AgregarAProcesoColaNuevos(t_pcbConConsola elemento){
	AgregarACola(elemento,&cola_PCBNuevos);
}

void AgregarAProcesoColaFinalizados(t_pcbConConsola elemento){
	AgregarACola(elemento,&cola_PCBFinalizados);
}

void AgregarAProcesoColaBloqueados(t_pcbConConsola elemento){
	AgregarACola(elemento,&cola_PCBBloqueados);
}
