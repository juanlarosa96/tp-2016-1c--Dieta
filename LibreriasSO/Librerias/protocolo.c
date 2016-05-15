/*
 * protocolo.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */
#include "protocolo.h"

int recibirHeader(int socketOrigen){
	int header;
	recv(socketOrigen, &header,sizeof(int),0);
	return header;
}

void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo){
	int header = programaAnsisop;
	send(socketDestino,&header,sizeof(int),0);
	send(socketDestino,&largoCodigo,sizeof(int),0);
	send(socketDestino,codigo,largoCodigo,0);
}

void recibirProgramaAnsisop(int socketOrigen, char * codigo){
	int largoCodigo;
	recibirTodo(socketOrigen,&largoCodigo,sizeof(int));
	recibirTodo(socketOrigen,codigo,&largoCodigo);
}
