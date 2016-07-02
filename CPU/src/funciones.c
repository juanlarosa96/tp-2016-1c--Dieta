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

	t_intructions* instrucciones_serializado = unPcb.indice_codigo.instrucciones;
	t_posicion_memoria posicionPagina;
	int tamanio = instrucciones_serializado[pc].offset;
	int numeroPagina = (instrucciones_serializado[pc].start) / tamanioPagina;
	int offset = (instrucciones_serializado[pc].start) % tamanioPagina;

	posicionPagina.offset = offset;
	posicionPagina.pagina = numeroPagina;
	posicionPagina.size = tamanio;
	return posicionPagina;
}

void recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina,
		char* lineaAnsisop) {
	recibirTodo(socketUMC, lineaAnsisop, posicionPagina.size);
}
int pedirLineaAUMC(int socketUMC, char * lineaAnsisop, t_pcb pcbActual,
		int tamanioPagina) {
	t_posicion_memoria posicion = obtenerPosicionPagina(tamanioPagina,
			pcbActual);
	return enviarPedidosDePosicionMemoria(socketUMC, posicion,
			(void *) lineaAnsisop, tamanioPagina);
}

int recibirBytesDePagina(int socketUMC, int largoPedido, void * buffer) {
	return recibirTodo(socketUMC, buffer, largoPedido);
}

int enviarPedidosDePosicionMemoria(int socketUMC, t_posicion_memoria posicion,
		void * buffer, int tamanioPagina) {
	int bytesTotales = posicion.offset + posicion.size;
	int bytesRecibidos = 0, offset = posicion.offset, pagina = posicion.pagina,
			tamanio = posicion.size;

	if (posicion.size + offset > tamanioPagina) {
		tamanio = tamanioPagina - offset;
	}

	while (bytesTotales >= tamanioPagina) {
		enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
		header = recibirHeader(socketUMC);
		if (header != pedidoMemoriaOK) {
			if (header == pedidoMemoriaFallo) {
				return 1;
			} else {
				log_error(logger, "Error conectando con UMC");
				abort();
			}
		}
		if (recibirBytesDePagina(socketUMC, tamanio,
				(void *) buffer + bytesRecibidos)) {
			log_error(logger, "Error conectando con UMC");
			abort();
		}
		bytesTotales -= tamanio;
		bytesTotales-=offset;
		bytesRecibidos += tamanio;
		tamanio = tamanioPagina;
		offset = 0;
		pagina++;
	}

	tamanio = bytesTotales;

	if (tamanio != 0) {
		enviarSolicitudDeBytes(socketUMC, pagina, offset, tamanio);
		header = recibirHeader(socketUMC);
		if (header != pedidoMemoriaOK) {
			if (header == pedidoMemoriaFallo) {
				return 1;
			} else {
				log_error(logger, "Error conectando con UMC");
				abort();
			}
		}
		if (recibirBytesDePagina(socketUMC, tamanio,
				(void *) buffer + bytesRecibidos)) {
			log_error(logger, "Error conectando con UMC");
			abort();
		}
	}
	return 0;
}

int enviarAlmacenamientosDePosicionMemoria(int socketUMC,
		t_posicion_memoria posicion, void * buffer, int tamanioPagina) {
	int bytesTotales = posicion.offset + posicion.size, header;
	int bytesEnviados = 0, offset = posicion.offset, pagina = posicion.pagina,
			tamanio = posicion.size;

	if (posicion.size + offset > tamanioPagina) {
		tamanio = tamanioPagina - offset;

	}

	while (bytesTotales >= tamanioPagina) {
		enviarPedidoAlmacenarBytes(socketUMC, pagina, offset, tamanio,
				(char *) buffer + bytesEnviados);
		header = recibirHeader(socketUMC);
		if (header != pedidoMemoriaOK) {
			if (header == pedidoMemoriaFallo) {
				return 1;
			} else {
				log_error(logger, "Error conectando con UMC");
				abort();
			}
		}
		bytesTotales -= tamanio;
		bytesTotales -= offset;
		bytesEnviados += tamanio;
		tamanio = tamanioPagina;
		offset = 0;
		pagina++;
	}

	tamanio = bytesTotales;

	if (tamanio != 0) {
		enviarPedidoAlmacenarBytes(socketUMC, pagina, offset, tamanio,
				(char *) buffer + bytesEnviados);
		header = recibirHeader(socketUMC);
		if (header != pedidoMemoriaOK) {
			if (header == pedidoMemoriaFallo) {
				return 1;
			} else {
				log_error(logger, "Error conectando con UMC");
				abort();
			}
		}
	}
	return 0;
}

void manejadorSIGUSR1(int signal_num) {
	if (signal_num == SIGUSR1) {
		log_info(logger, "Llegó la señal SIGUSR1.");
		signalApagado = 1;
		sem_post(&semComenzarQuantum);
	}

}

void avisarANucleoFinalizacionDeCPU(int socketNucleo) {
	enviarSenialDeApagadoDeCPU(socketNucleo);

}

void hiloSignalYHeader() {
	while (1) {
		sem_wait(&semRecibirHeader);
		header = recibirHeader(socketNucleo);
		sem_post(&semComenzarQuantum);
	}

}

void borrarBarraTesYEnesDeString(char* variable) {
	int i = 0;
	while (variable[i] != '\0') {
		if ((variable[i] == '\t') || (variable[i] == '\n')) {
			variable[i] = '\0';
			i--;
		}
		i++;
	}
}

void destruirPcb(t_pcb pcb) {
	free(pcb.indice_codigo.instrucciones);
	free(pcb.indice_etiquetas.etiquetas);
	list_destroy_and_destroy_elements(pcb.indice_stack, (void *) destruirRegistroStack);
}

void destruirRegistroStack(t_registro_pila * registro) {
	list_destroy_and_destroy_elements(registro->lista_argumentos, free);
	list_destroy_and_destroy_elements(registro->lista_variables, free);
}
