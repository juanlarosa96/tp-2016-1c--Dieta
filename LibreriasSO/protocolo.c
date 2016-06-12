/*
 * protocolo.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */
#include "protocolo.h"
#include "sockets.h"

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
	int header = tamanioDePagina;
	send(socketDestino, &header, sizeof(int), 0);
	send(socketDestino, &tamanioPagina, sizeof(int), 0);

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

void enviarInicializacionPrograma(int socketUMC, uint32_t pid, int largoCodigo, char * programa, uint32_t paginasRequeridas) {
	int header = iniciarPrograma;
	send(socketUMC, &header, sizeof(int), 0);
	void * buffer = malloc(sizeof(uint32_t) * 2 + sizeof(int) + largoCodigo);
	int cursorMemoria = 0;

	memcpy(buffer, &pid, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &paginasRequeridas, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &largoCodigo, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(buffer + cursorMemoria, programa, largoCodigo);
	cursorMemoria += largoCodigo;
	send(socketUMC, buffer, cursorMemoria, 0);
	free(buffer);
}

void recibirInicializacionPrograma(int socketUMC, uint32_t *pid, uint32_t *paginasRequeridas, int *largoCodigo) {
	recibirTodo(socketUMC, pid, sizeof(uint32_t));
	recibirTodo(socketUMC, paginasRequeridas, sizeof(uint32_t));
	recibirTodo(socketUMC, largoCodigo, sizeof(int));
}

void recibirCodigoInicializarPrograma(int socketUMC, int largoCodigo, char *codigo) {
	recibirTodo(socketUMC, codigo, largoCodigo);
}

int recibirRespuestaInicializacion(int socketUMC) {
	int respuesta;
	recibirTodo(socketUMC, &respuesta, sizeof(int));
	return respuesta;

}

void enviarRespuestaInicializacionExito(int socketDestino) {
	int header = inicioProgramaExito;
	send(socketDestino, &header, sizeof(int), 0);
}

void enviarRespuestaInicializacionError(int socketDestino) {
	int header = inicioProgramaError;
	send(socketDestino, &header, sizeof(int), 0);
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

void enviarPedidoAlmacenarBytes(int socketUMC, uint32_t nroPagina, uint32_t offset, uint32_t size, void * bufferA) {

	int header = almacenarBytes;
	send(socketUMC, &header, sizeof(int), 0);

	void * bufferPedido = malloc(sizeof(int) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + size); //largoBuffer con el barra cero? YES, que cpu lo ponga
	int cursorMemoria = 0;

	memcpy(bufferPedido, &nroPagina, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, &offset, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, &size, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(bufferPedido, bufferA, size);
	cursorMemoria += size;
	send(socketUMC, bufferPedido, cursorMemoria, 0);

	free(bufferPedido);
}

void recibirPedidoAlmacenarBytes(int socketUMC, uint32_t *nroPagina, uint32_t *offset, uint32_t *size) {
	recibirTodo(socketUMC, nroPagina, sizeof(uint32_t));
	recibirTodo(socketUMC, offset, sizeof(uint32_t));
	recibirTodo(socketUMC, size, sizeof(uint32_t));
}

void recibirBufferPedidoAlmacenarBytes(int socketUMC, int largoPedido, char * buffer) {
	recibirTodo(socketUMC, buffer, largoPedido);
}

void enviarValorAImprimir(int socketNucleo, uint32_t id_proceso, char * texto) {

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

	send(socketNucleo, data, offset, 0);

	free(data);

}

void recibirValorAImprimir(int socketOrigen, uint32_t *id_proceso, int *largoTexto, char ** texto) {
	recibirTodo(socketOrigen, id_proceso, sizeof(uint32_t));
	recibirTodo(socketOrigen, largoTexto, sizeof(int));
	*texto = malloc(*largoTexto);
	recibirTodo(socketOrigen, texto, *largoTexto);

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
		memcpy(buffer + cursorMemoria, &cantidadeElementosLista, sizeof(int));
		cursorMemoria += sizeof(int);

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
		memcpy(buffer + cursorMemoria, &cantidadeElementosLista, sizeof(int));
		cursorMemoria += sizeof(int);

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

		memcpy(buffer + cursorMemoria, &(registro->posicionUltimaVariable), sizeof(uint32_t));
		cursorMemoria += sizeof(uint32_t);
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

t_pcb recibirPcb(int socketNucleo) {
	t_pcb pcb;
	int largo32 = sizeof(uint32_t), i, j, cantidadElementosStack;
	recibirTodo(socketNucleo, &(pcb.pid), largo32);
	recibirTodo(socketNucleo, &(pcb.pc), largo32);
	recibirTodo(socketNucleo, &(pcb.paginas_codigo), largo32);
	recibirTodo(socketNucleo, &(pcb.indice_codigo.cantidadInstrucciones), largo32);

	pcb.indice_codigo.instrucciones = malloc(sizeof(t_intructions) * pcb.indice_codigo.cantidadInstrucciones);

	for (i = 0; i < pcb.indice_codigo.cantidadInstrucciones; i++) {

		t_intructions instruccion;
		recibirTodo(socketNucleo, &(instruccion.start), largo32);
		recibirTodo(socketNucleo, &(instruccion.offset), largo32);
		pcb.indice_codigo.instrucciones[i] = instruccion;

	}
	recibirTodo(socketNucleo, &(pcb.indice_codigo.numeroInstruccionInicio), largo32);

	recibirTodo(socketNucleo, &(pcb.indice_etiquetas.largoTotalEtiquetas), largo32);
	recibirTodo(socketNucleo, &(pcb.indice_etiquetas.etiquetas), pcb.indice_etiquetas.largoTotalEtiquetas);

	recibirTodo(socketNucleo, &cantidadElementosStack, sizeof(int));

	t_list *pilaAuxiliar = list_create();
	for (i = 0; i < cantidadElementosStack; i++) {

		t_registro_pila elementoPila;
		int cantidadElementosLista;

		recibirTodo(socketNucleo, &(elementoPila.direccion_retorno), largo32);

		recibirTodo(socketNucleo, &cantidadElementosLista, sizeof(int));

		for (j = 0; j < cantidadElementosLista; j++) {

			t_posicion_memoria elementoLista;

			recibirTodo(socketNucleo, &(elementoLista.pagina), largo32);
			recibirTodo(socketNucleo, &(elementoLista.offset), largo32);
			recibirTodo(socketNucleo, &(elementoLista.size), largo32);

			list_add(elementoPila.lista_argumentos, &elementoLista);

		}

		recibirTodo(socketNucleo, &cantidadElementosLista, sizeof(int));

		for (j = 0; j < cantidadElementosLista; j++) {

			t_identificadorConPosicionMemoria elementoLista;

			recibirTodo(socketNucleo, &(elementoLista.identificador), sizeof(char));
			recibirTodo(socketNucleo, &(elementoLista.posicionDeVariable.pagina), largo32);
			recibirTodo(socketNucleo, &(elementoLista.posicionDeVariable.offset), largo32);
			recibirTodo(socketNucleo, &(elementoLista.posicionDeVariable.size), largo32);

			list_add(elementoPila.lista_variables, &elementoLista);

		}
		recibirTodo(socketNucleo, &(elementoPila.posicionUltimaVariable), largo32);
		recibirTodo(socketNucleo, &(elementoPila.variable_retorno.pagina), largo32);
		recibirTodo(socketNucleo, &(elementoPila.variable_retorno.offset), largo32);
		recibirTodo(socketNucleo, &(elementoPila.variable_retorno.size), largo32);

		pushPila(pilaAuxiliar, &elementoPila);

	}

	//Elementos de pila auxiliar estan en orden inverso
	for (i = 0; i < cantidadElementosStack; i++) {

		t_registro_pila *registro = popPila(pilaAuxiliar);
		pushPila(pcb.indice_stack, registro);
		free(registro);
	}

	return pcb;
}

void enviarFinalizacionProgramaUMC(int socketUMC, uint32_t pid) {
	int header = finalizacionPrograma;

	void * data = malloc(sizeof(int) + sizeof(uint32_t));
	int offset = 0, size = 0;

	size = sizeof(int);
	memcpy(data, &header, size);
	offset += size;
	size = sizeof(uint32_t);
	memcpy(data + offset, &pid, size);
	offset += size;

	send(socketUMC, data, offset, 0);

	free(data);
}

void enviarCambioProcesoActivo(int socketUMC, uint32_t pid) {
	int header = cambiarProcesoActivo;

	void * data = malloc(sizeof(int) + sizeof(uint32_t));
	int offset = 0, size = 0;

	size = sizeof(int);
	memcpy(data, &header, size);
	offset += size;
	size = sizeof(uint32_t);
	memcpy(data + offset, &pid, size);
	offset += size;

	send(socketUMC, data, offset, 0);

	free(data);
}

void recibirPID(int socketUMC, uint32_t * pid) {
	recibirTodo(socketUMC, pid, sizeof(uint32_t));
}

void enviarEntradaSalida(int socketNucleo, t_pcb pcb, t_nombre_dispositivo dispositivo, int tiempo) {
	int header = headerEntradaSalida;

	enviarPcb(socketNucleo, pcb);

	void *data = malloc(sizeof(int) + sizeof(int) + strlen(dispositivo) + 1 + sizeof(int)); //header + pid + largoNombreDispositivo + nombreDispositivo + tiempo
	int offset = 0, str_size = 0, largoNombreDispositivo = strlen(dispositivo) + 1;

	str_size = sizeof(int);
	memcpy(data + offset, &header, str_size);
	offset += str_size;

	str_size = sizeof(int);
	memcpy(data + offset, &largoNombreDispositivo, str_size);
	offset += str_size;

	str_size = strlen(dispositivo) + 1;
	memcpy(data + offset, &dispositivo, str_size);
	offset += str_size;

	str_size = sizeof(int);
	memcpy(data + offset, &tiempo, str_size);
	offset += str_size;

	send(socketNucleo, data, offset, 0);

	free(data);

}

void recibirEntradaSalida(int socketOrigen, int *largoNombreDispositivo, char ** nombreDispositivo, int *tiempo) {
	recibirTodo(socketOrigen, largoNombreDispositivo, sizeof(int));
	*nombreDispositivo = malloc(*largoNombreDispositivo);
	recibirTodo(socketOrigen, nombreDispositivo, *largoNombreDispositivo);
	recibirTodo(socketOrigen, tiempo, sizeof(int));
}

void enviarFinalizacionProgramaConsola(int socketConsola) {
	int header = finalizacionPrograma;
	send(socketConsola, &header, sizeof(int), 0);

}

void enviarPaginasRequeridasASwap(int socketSwap, int paginasRequeridas) {
	int header = inicializarProgramaSwap;
	void * data = malloc(sizeof(int) + sizeof(int));
	int offset = 0, size = 0;

	size = sizeof(int);
	memcpy(data, &header, size);

	offset += size;
	memcpy(data + offset, &paginasRequeridas, size);

	offset += size;

	send(socketSwap, data, offset, 0);
}

void enviarWait(int socketNucleo, int id_proceso, t_nombre_semaforo nombreSemaforo) {
	int header = headerWait;

	void *data = malloc(sizeof(int) + sizeof(uint32_t) + sizeof(int) + strlen(nombreSemaforo) + 1); //header + pid + largoNombreSemaforo + nombreSemaforo
	int offset = 0, str_size = 0, largoNombreSemaforo = strlen(nombreSemaforo) + 1;

	str_size = sizeof(int);
	memcpy(data + offset, &header, str_size);
	offset += str_size;

	str_size = sizeof(uint32_t);
	memcpy(data + offset, &id_proceso, str_size);
	offset += str_size;

	str_size = sizeof(int);
	memcpy(data + offset, &largoNombreSemaforo, str_size);
	offset += str_size;

	str_size = strlen(nombreSemaforo) + 1;
	memcpy(data + offset, &nombreSemaforo, str_size);
	offset += str_size;

	send(socketNucleo, data, offset, 0);

	free(data);

}

void recibirWait(int socketOrigen, uint32_t *id_proceso, int *largoNombreSemaforo, t_nombre_semaforo * nombreSemaforo) {
	recibirTodo(socketOrigen, id_proceso, sizeof(uint32_t));
	recibirTodo(socketOrigen, largoNombreSemaforo, sizeof(int));
	recibirTodo(socketOrigen, nombreSemaforo, *largoNombreSemaforo);
}

void enviarSignal(int socketNucleo, int id_proceso, t_nombre_semaforo nombreSemaforo) {
	int header = headerSignal;

	void *data = malloc(sizeof(int) + sizeof(uint32_t) + sizeof(int) + strlen(nombreSemaforo) + 1); //header + pid + largoNombreSemaforo + nombreSemaforo
	int offset = 0, str_size = 0, largoNombreSemaforo = strlen(nombreSemaforo) + 1;

	str_size = sizeof(int);
	memcpy(data + offset, &header, str_size);
	offset += str_size;

	str_size = sizeof(uint32_t);
	memcpy(data + offset, &id_proceso, str_size);
	offset += str_size;

	str_size = sizeof(int);
	memcpy(data + offset, &largoNombreSemaforo, str_size);
	offset += str_size;

	str_size = strlen(nombreSemaforo) + 1;
	memcpy(data + offset, &nombreSemaforo, str_size);
	offset += str_size;

	send(socketNucleo, data, offset, 0);

	free(data);

}

void recibirSignal(int socketOrigen, uint32_t *id_proceso, int *largoNombreSemaforo, t_nombre_semaforo * nombreSemaforo) {
	recibirTodo(socketOrigen, id_proceso, sizeof(uint32_t));
	recibirTodo(socketOrigen, largoNombreSemaforo, sizeof(int));
	recibirTodo(socketOrigen, nombreSemaforo, *largoNombreSemaforo);
}

int recibirCantidadQuantum(int socketOrigen) {
	int cantidad;
	int bytesRecibidos;
	if ((bytesRecibidos = recv(socketOrigen, &cantidad, sizeof(int), 0)) <= 0) {
		return bytesRecibidos;
	} else {
		return cantidad;
	}
}

void enviarUnidadesQuantum(int socketCPU, int unidades) {
	int header = quantumUnidades;
	void * data = malloc(sizeof(int) + sizeof(int));
	memcpy(data, &header, sizeof(int));
	memcpy(data + sizeof(int), &unidades, sizeof(int));
	send(socketCPU, data, sizeof(int) * 2, 0);
	free(data);
}

void enviarSleepQuantum(int socketCPU, int sleep) {
	int header = quantumSleep;
	void * data = malloc(sizeof(int) + sizeof(int));
	memcpy(data, &header, sizeof(int));
	memcpy(data + sizeof(int), &sleep, sizeof(int));
	send(socketCPU, data, sizeof(int) * 2, 0);
	free(data);
}

void enviarFinalizacionProgramaNucleo(int socketNucleo) {
	int header = finalizacionPrograma;
	send(socketNucleo, &header, sizeof(int), 0);
}

void enviarAbortarProgramaNucleo(int socketNucleo) {
	int header = abortarPrograma;
	send(socketNucleo, &header, sizeof(int), 0);
}

void pedirCompartidaNucleo(int socketNucleo, char variable, int * punteroVariable){
	int header = pedidoVariableCompartida;
	void * data = malloc(sizeof(int) + sizeof(char));
	memcpy(data, &header, sizeof(int));
	memcpy(data + sizeof(int), &variable, sizeof(char));
	send(socketNucleo, data, sizeof(int)+sizeof(char), 0);
	free(data);
	recibirTodo(socketNucleo, (void *) punteroVariable, sizeof(int));
}

void asignarCompartidaNucleo(int socketNucleo, char variable, int valor){
	int header = asignacionVariableCompartida;
	void * data = malloc(sizeof(int) *2 + sizeof(char));
	memcpy(data, &header, sizeof(int));
	memcpy(data + sizeof(int), &variable, sizeof(char));
	memcpy(data + sizeof(int) + sizeof(char), &valor, sizeof(int));
	send(socketNucleo, data, sizeof(int)*2 +sizeof(char), 0);
	free(data);
}

void pedirPaginaASwap(int socketSwap, uint32_t pid, int nroPagina){ //Sofi comment: chequearla!
	int header = pedidoPaginaASwap;
	int offset = 0;
	void * data = malloc(sizeof(int) *2 + sizeof(uint32_t));
	memcpy(data, &header, sizeof(int));
	offset += sizeof(int);
	memcpy(data + offset, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(data + offset, &nroPagina, sizeof(int));
	offset += sizeof(int);
	send(socketSwap, data, offset, 0);
	free(data);
}

void enviarAbortarProceso(int socketCPU){
	int header = pedidoMemoriaFallo;
	send(socketCPU, &header, sizeof(int), 0);

}

void enviarPedidoMemoriaOK(int socketCPU){
	int header = pedidoMemoriaOK;
	send(socketCPU, &header, sizeof(int), 0);
}
