/*
 * funciones.c
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */
#include "funciones.h"

int iniciarProgramaAnsisop(int cliente, char*archivo) {

	t_proceso * proceso = malloc(sizeof(t_proceso));

	int cantPaginasTotal;
	recibirTodo(cliente, &cantPaginasTotal, sizeof(int));

	int frameInicial= chequearMemoriaDisponible(cantPaginasTotal,archivo);
	if (frameInicial == -1) {
		avisarUMCFallo(cliente);
		return 0;
	} else {
		avisarUMCExito(cliente);
	}

	uint32_t pid;
	recibirTodo(cliente, &pid, sizeof(uint32_t));

	int cantPaginasCodigo;
	recibirTodo(cliente, &cantPaginasCodigo, sizeof(int));

	int cantPaginasStack = cantPaginasTotal - cantPaginasCodigo;

	proceso->pID = pid;
	proceso->frameInicial = frameInicial;
	proceso->cantPaginas = cantPaginasTotal;
	proceso->cantPaginasCodigo = cantPaginasCodigo;
	proceso->cantPaginasStack = cantPaginasStack;

	list_add(listaProcesos,proceso);
	int i;
	char *pagina = malloc(sizePagina);

	for (i = 0; i < cantPaginasTotal; i++) {
		if(i < cantPaginasCodigo){

		recibirTodo(cliente, pagina, sizePagina);
		archivo[frameInicial] = *pagina;

		}

		bitMap[frameInicial] = 1;
		frameInicial ++;
	}
	free(pagina);
	return 1;
}

void guardarPaginas(int cliente,char*archivo){

	int nroPagina;
	recibirTodo(cliente,&nroPagina,sizeof(int));

	uint32_t pID;
	recibirTodo(cliente,&pID,sizeof(uint32_t));

	char *pagina = malloc(sizePagina);
	recibirTodo(cliente,pagina,sizeof(sizePagina));

	int i;
	t_proceso *procesoAux;
	for(i = 0 ; i < list_size(listaProcesos) ; i++){
		procesoAux = list_get(listaProcesos,i);

		if (procesoAux->pID == pID ){
			archivo[procesoAux->frameInicial + nroPagina] = *pagina;
			break;
		}

	}
}

void enviarPaginas(int cliente,char*archivo){
	uint32_t pID;
	recibirTodo(cliente,&pID,sizeof(uint32_t));

	int nroPagina;
	recibirTodo(cliente,&nroPagina,sizeof(int));

	int i;
		t_proceso *procesoAux;
		for(i = 0 ; i < list_size(listaProcesos) ; i++){
			procesoAux = list_get(listaProcesos,i);

			if (procesoAux->pID == pID ){
				send(cliente,&(archivo[procesoAux->frameInicial + nroPagina]),sizePagina,0);
				break;
			}

		}


}

void finalizarProgramaAnsisop(cliente){

	uint32_t pID;
	recibirTodo(cliente,&pID,sizeof(uint32_t));

	int i;
	t_proceso *procesoAux;
	for(i = 0 ; i < list_size(listaProcesos) ; i++){
	procesoAux = list_get(listaProcesos,i);
	if (procesoAux->pID == pID ){
		int a;
		for(a=0;a<procesoAux->cantPaginas;a++){
			bitMap[a+procesoAux->frameInicial]=0;
		}
	    free(list_remove(listaProcesos,i));
				}
	}
}

int chequearMemoriaDisponible(int cantPaginas,char*archivo) {
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
		i++;
	}

	if(hayTotales){
			if(hayContinuas){
				return (i-1)-cantPaginas;
			} else{
			return compactar(archivo);
			}
	} else {
	return -1;
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

int compactar(char*archivo){
	int i;
	int ultimoFrameLibre = 0;
	for(i=0;i<cantidadDeFrames-1;i++){
		if(bitMap[i]==0 && bitMap[i+1]==1){
			archivo[i] = archivo[i+1];
			bitMap[i]=1;
			bitMap[i+1]=0;
			i = ultimoFrameLibre -1;
		}
		if(bitMap[i]==1){
			ultimoFrameLibre++;
		}
	}
	sleep(retardoCompactacion);
	return ultimoFrameLibre;
}

