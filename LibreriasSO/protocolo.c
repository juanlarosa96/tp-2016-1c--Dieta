/*
 * protocolo.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */
#include "protocolo.h"

int recibirHeader(int socketOrigen) {
	int header;
	int bytesRecibidos;
	if ((bytesRecibidos = recv(socketOrigen, &header, sizeof(int), 0)) <= 0) {
		return bytesRecibidos;
	} else {
		return header;
	}
}

void enviarProgramaAnsisop(int socketDestino, char * codigo, int largoCodigo) {
	int header = programaAnsisop;
	send(socketDestino, &header, sizeof(int), 0);
	send(socketDestino, &largoCodigo, sizeof(int), 0);
	send(socketDestino, codigo, largoCodigo, 0); //hay que serializar algo acá?
}

void recibirProgramaAnsisop(int socketOrigen, char * codigo, int largoCodigo) {
	recibirTodo(socketOrigen, codigo, &largoCodigo);
}

int recibirLargoProgramaAnsisop(int socketOrigen) {
	int largoCodigo;
	recibirTodo(socketOrigen, &largoCodigo, sizeof(int));
	return largoCodigo;
}

int recibirTamanioPagina(int socketOrigen) {
	int tamanio;
	recibirTodo(socketOrigen, &tamanio, sizeof(int));
	return tamanio;

}

void enviarTamanioPagina(int socketDestino, int tamanioPagina){
	char temporal[5];
	sprintf(temporal, "%d", tamanioPagina);
	char buffer[2 + 5]; //header + payload
	strcpy(buffer, "3"); //header, protocolo
	strcat(buffer, temporal);
	int len = strlen(buffer);
	send(socketDestino, buffer, len, 0); //implementado de forma horrenda

}

t_pcb recibirPcb(int socketOrigen) {
	t_pcb pcbNuevo;
	recibirTodo(socketOrigen, &pcbNuevo, sizeof(t_pcb));
	return pcbNuevo;
}

void enviarPedidoPaginas(int socketUMC, int cantidadPaginas){
	int header = programaAnsisop;
		send(socketUMC, &header, sizeof(int), 0);
		send(socketUMC, &cantidadPaginas, sizeof(int), 0);
}

void recibirResultadoDeEjecucionAnsisop(int socketNucleo, char * mensaje, int largoMensaje) {
	recibirTodo(socketNucleo, mensaje, &largoMensaje);
}

int recibirLargoResultadoDeEjecucionAnsisop(int socketNucleo) {
	int largoMensaje;
	recibirTodo(socketNucleo, &largoMensaje, sizeof(int));
	return largoMensaje;
}

void enviarResultadoDeEjecucionAnsisop(int socketDestino, char * mensaje, int largoMensaje) {
	int header = resultadoEjecucion;
	send(socketDestino, &header, sizeof(int), 0);
	send(socketDestino, &largoMensaje, sizeof(int), 0);
	send(socketDestino, mensaje, largoMensaje, 0);
}
