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
	recibirTodo(socketOrigen, codigo, largoCodigo);
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

void recibirResultadoDeEjecucionAnsisop(int socketNucleo, char * mensaje, int largoMensaje) {
	recibirTodo(socketNucleo, mensaje, largoMensaje);
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

int recibirRespuestaCPU(int socketCpu, int * respuesta) {
	return recibirTodo(socketCpu, respuesta, sizeof(int));
}

void enviarInicializacionPrograma(int socketUMC, uint32_t pid, int largoPrograma, char * programa, uint32_t paginas_codigo) {
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

void recibirInicializacionPrograma(int socketUMC, uint32_t *pid, int* largoPrograma, char * programa, uint32_t *paginas_codigo) {
	recibirTodo(socketUMC, pid, sizeof(uint32_t));
	recibirTodo(socketUMC, paginas_codigo, sizeof(uint32_t));
	recibirTodo(socketUMC, largoPrograma, sizeof(int));
	programa = malloc(*largoPrograma);
	recibirTodo(socketUMC, programa, *largoPrograma);
}

int recibirRespuestaInicializacion(int socketUMC) { //soy sofi y no entiendo el porque de esta funcion
	int respuesta;
	recibirTodo(socketUMC, &respuesta, sizeof(int));
	return respuesta;

}

void enviarSolicitudDeBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size) {
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

void recibirSolicitudDeBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size) {
	recibirTodo(socketUMC, nroPagina, sizeof(uint32_t));
	recibirTodo(socketUMC, offset, sizeof(uint32_t));
	recibirTodo(socketUMC, size, sizeof(uint32_t));
}

void enviarPedidoAlmacenarBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size, int largoBuffer, char * bufferA) {

	int header = almacenarBytes;
	send(socketUMC, &header, sizeof(int), 0);

	void * bufferPedido = malloc(sizeof(int) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int) + largoBuffer); //largoBuffer con el barra cero? YES, que cpu lo ponga
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

void recibirPedidoAlmacenarBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size, int * largoBuffer) {
	recibirTodo(socketUMC, nroPagina, sizeof(uint32_t));
	recibirTodo(socketUMC, offset, sizeof(uint32_t));
	recibirTodo(socketUMC, size, sizeof(uint32_t));
	recibirTodo(socketUMC, largoBuffer, sizeof(int));
}

void recibirBufferPedidoAlmacenarBytes(int socketUMC, int largoPedido, char * buffer) {
	recibirTodo(socketUMC, buffer, largoPedido);
}

void enviarPcb(int socketCPU, t_pcb pcb) {

	int header = headerPcb;
	send(socketCPU, &header, sizeof(int), 0);

	int cantidadElementosStack = 0, cursorMemoria = 0, i = 0;

	void* buffer = malloc(1000);

	memcpy(buffer, &(pcb.pid), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	memcpy(buffer + cursorMemoria, &(pcb.pc), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	memcpy(buffer + cursorMemoria, &(pcb.paginas_codigo), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	memcpy(buffer + cursorMemoria, &(pcb.indice_codigo.cantidadInstrucciones), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	for (i = 0; i < pcb.indice_codigo.cantidadInstrucciones; i++) {

		memcpy(buffer + cursorMemoria, &(pcb.indice_codigo.instrucciones[i].start), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);

		memcpy(buffer + cursorMemoria, &(pcb.indice_codigo.instrucciones[i].offset), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);
	}

	memcpy(buffer + cursorMemoria, &(pcb.indice_codigo.numeroInstruccionInicio), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	memcpy(buffer + cursorMemoria, &(pcb.indice_etiquetas.largoTotalEtiquetas), sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	memcpy(buffer + cursorMemoria, &(pcb.indice_etiquetas.etiquetas), pcb.indice_etiquetas.largoTotalEtiquetas);
	cursorMemoria += pcb.indice_etiquetas.largoTotalEtiquetas;

	cantidadElementosStack = list_size(pcb.indice_stack);
	memcpy(buffer + cursorMemoria, &cantidadElementosStack, sizeof(int));
	cursorMemoria += sizeof(int);

	for (i = 0; i < cantidadElementosStack; i++) {

		t_registro_pila * registro = popPila(pcb.indice_stack);
		memcpy(buffer + cursorMemoria, &(registro->direccion_retorno), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);

		int j, cantidadeElementosLista;
		cantidadeElementosLista = list_size(registro->lista_argumentos);
		for (j = 0; j < cantidadeElementosLista; j++) {

			t_posicion_memoria * elementoLista = (t_posicion_memoria *) list_remove(registro->lista_argumentos, 0);

			memcpy(buffer + cursorMemoria, &(elementoLista->pagina), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(buffer + cursorMemoria, &(elementoLista->offset), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(buffer + cursorMemoria, &(elementoLista->size), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
		}

		cantidadeElementosLista = list_size(registro->lista_variables);
		for (j = 0; j < cantidadeElementosLista; j++) {

			t_identificadorConPosicionMemoria* elementoLista = (t_identificadorConPosicionMemoria *) list_remove(registro->lista_variables, 0);

			memcpy(buffer + cursorMemoria, &(elementoLista->identificador), sizeof(char));
			cursorMemoria += sizeof(char);
			memcpy(buffer + cursorMemoria, &(elementoLista->posicionDeVariable.pagina), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(buffer + cursorMemoria, &(elementoLista->posicionDeVariable.offset), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(buffer + cursorMemoria, &(elementoLista->posicionDeVariable.size), sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
		}

		memcpy(buffer + cursorMemoria, &(registro->variable_retorno.pagina), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);
		memcpy(buffer + cursorMemoria, &(registro->variable_retorno.offset), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);
		memcpy(buffer + cursorMemoria, &(registro->variable_retorno.size), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);
	}

	send(socketCPU, buffer, cursorMemoria, 0);
	free(buffer);
}
