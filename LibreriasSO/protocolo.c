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

void enviarTamanioPagina(int socketDestino, int tamanioPagina) {
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

void recibirResultadoDeEjecucionAnsisop(int socketNucleo, char * mensaje,
		int largoMensaje) {
	recibirTodo(socketNucleo, mensaje, &largoMensaje);
}

int recibirLargoResultadoDeEjecucionAnsisop(int socketNucleo) {
	int largoMensaje;
	recibirTodo(socketNucleo, &largoMensaje, sizeof(int));
	return largoMensaje;
}

void enviarResultadoDeEjecucionAnsisop(int socketDestino, char * mensaje,
		int largoMensaje) {
	int header = resultadoEjecucion;
	send(socketDestino, &header, sizeof(int), 0);
	send(socketDestino, &largoMensaje, sizeof(int), 0);
	send(socketDestino, mensaje, largoMensaje, 0);
}

int recibirRespuestaCPU(int socketCpu, int * respuesta) {
	return recibirTodo(socketCpu, respuesta, sizeof(int));
}

void enviarInicializacionPrograma(int socketUMC, uint32_t pid,
		int largoPrograma, char * programa, uint32_t paginas_codigo) {
	int header = iniciarPrograma;
	send(socketUMC, &header, sizeof(int), 0);
	void * buffer = malloc(sizeof(uint32_t) * 2 + sizeof(int) + largoPrograma);
	int cursorMemoria = 0;

	memcpy(buffer, &pid, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &paginas_codigo, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &largoPrograma, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(buffer + cursorMemoria, programa, largoPrograma);
	cursorMemoria += largoPrograma;
	send(socketUMC, buffer, cursorMemoria, 0);
	free(buffer);
}

void recibirInicializacionPrograma(int socketUMC, uint32_t *pid,
		int* largoPrograma, char * programa, uint32_t *paginas_codigo) {
	recibirTodo(socketUMC, pid, sizeof(uint32_t));
	recibirTodo(socketUMC, paginas_codigo, sizeof(uint32_t));
	recibirTodo(socketUMC, largoPrograma, sizeof(int));
	programa = malloc(*largoPrograma);
	recibirTodo(socketUMC, programa, largoPrograma);
}

int recibirRespuestaInicializacion(int socketUMC) {
	int respuesta;
	recibirTodo(socketUMC, &respuesta, sizeof(int));
	return respuesta;

}

void enviarSolicitudDeBytes(int socketUMC, uint32_t nroPagina, uint32_t offset,
		uint32_t size) {
	int header = solicitarBytes;
	send(socketUMC, &header, sizeof(int), 0);
	void * buffer = malloc(sizeof(uint32_t) * 3);
	int cursorMemoria = 0;

	memcpy(buffer, &nroPagina, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer, &offset, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer, &size, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	send(socketUMC, buffer, cursorMemoria, 0);

	free(buffer);
}

void recibirSolicitudDeBytes(int socketUMC, uint32_t *nroPagina,
		uint32_t *offset, uint32_t *size) {
	recibirTodo(socketUMC, nroPagina, sizeof(uint32_t));
	recibirTodo(socketUMC, offset, sizeof(uint32_t));
	recibirTodo(socketUMC, size, sizeof(uint32_t));
}

void enviarPedidoAlmacenarBytes(int socketUMC, uint32_t nroPagina,
		uint32_t offset, uint32_t size, int largoBuffer, char * bufferA) {

	int header = almacenarBytes;
	send(socketUMC, &header, sizeof(int), 0);

	void * bufferPedido = malloc(
			sizeof(int) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)
					+ sizeof(int) + largoBuffer); //largoBuffer con el barra cero? YES, que cpu lo ponga
	int cursorMemoria = 0;

	memcpy(bufferPedido, &nroPagina, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, &offset, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, &size, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, &largoBuffer, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(bufferPedido, bufferA, largoBuffer);
	cursorMemoria += largoBuffer;
	send(socketUMC, bufferPedido, cursorMemoria, 0);

	free(bufferPedido);
}

void recibirPedidoAlmacenarBytes(int socketUMC, uint32_t *nroPagina,
		uint32_t *offset, uint32_t *size, int * largoBuffer) {
	recibirTodo(socketUMC, nroPagina, sizeof(uint32_t));
	recibirTodo(socketUMC, offset, sizeof(uint32_t));
	recibirTodo(socketUMC, size, sizeof(uint32_t));
	recibirTodo(socketUMC, largoBuffer, sizeof(int));
}

void recibirBufferPedidoAlmacenarBytes(int socketUMC, int largoPedido, char * buffer){
	recibirTodo(socketUMC, buffer, largoPedido);
}

void enviarValorAImprimir(int socketNucleo, uint32_t id_proceso, char * texto){

	int header = primitivaImprimir;

	void *data = malloc(sizeof(int) + sizeof(uint32_t) + sizeof(int) + strlen(texto) + 1); //header + pid + largoTexto + texto
	int offset = 0, str_size = 0, largoTexto = strlen(texto) + 1;

	    str_size = sizeof(int);
		memcpy(data + offset, &header, str_size);
		offset += str_size;

		str_size = sizeof(uint32_t);
		memcpy(data + offset, &id_proceso, str_size);
		offset += str_size;

		str_size = sizeof(int);
		memcpy(data + offset, &largoTexto, str_size);
		offset += str_size;

		str_size = strlen(texto) + 1;
		memcpy(data + offset, texto, str_size);
		offset += str_size;

		send(socketNucleo,data,offset,0);

		free(data);


}

void recibirValorAImprimir(int socketOrigen, uint32_t *id_proceso, int *largoTexto, char * texto){
	recibirTodo(socketOrigen, id_proceso, sizeof(uint32_t));
	recibirTodo(socketOrigen, largoTexto, sizeof(int));
	recibirTodo(socketOrigen, texto, *largoTexto);

}

