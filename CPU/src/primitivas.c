/*
 * primitivas.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable variable) {
	t_posicion_memoria * posicionVariable = malloc(sizeof(t_posicion_memoria));
	t_registro_pila *regPila = popPila(pcbRecibido.indice_stack);
	posicionVariable->pagina = regPila->posicionUltimaVariable / tamanioPagina;
	posicionVariable->offset = regPila->posicionUltimaVariable % tamanioPagina;
	posicionVariable->size = TAM_VAR;
	regPila->posicionUltimaVariable += TAM_VAR;

	int aux = variable - '0';
	if (aux >= 0 && aux <= 9) {
		list_add(regPila->lista_argumentos, posicionVariable);
	} else {
		t_identificadorConPosicionMemoria * nuevaVariable = malloc(sizeof(t_identificadorConPosicionMemoria));
		nuevaVariable->identificador = variable;
		nuevaVariable->posicionDeVariable = *posicionVariable;
		list_add(regPila->lista_variables, nuevaVariable);
		free(posicionVariable);
	}

	pushPila(pcbRecibido.indice_stack, regPila);
	log_info(logger, "Se definio la variable %c", variable);
	return regPila->posicionUltimaVariable - TAM_VAR;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	/*
	 * me pasan un identif y me fijo en el stack de cpu cual es la pos
	 */
	t_puntero posicion = -1;
	int argumento = variable - '0';
	t_registro_pila *regPila = popPila(pcbRecibido.indice_stack);

	if (argumento >= 0 && argumento <= 9) {
		t_posicion_memoria * posicionArgumento = (t_posicion_memoria *) list_get(regPila->lista_argumentos, argumento);
		posicion = posicionArgumento->pagina * tamanioPagina + posicionArgumento->offset;
	} else {
		int largoLista = list_size(regPila->lista_variables);
		int i = 0;
		t_identificadorConPosicionMemoria * elementoLista;
		for (; i < largoLista; i++) {
			elementoLista = list_get(regPila->lista_variables, i);
			if (elementoLista->identificador == variable) {
				posicion = elementoLista->posicionDeVariable.pagina * tamanioPagina + elementoLista->posicionDeVariable.offset;
			}
		}
	}
	pushPila(pcbRecibido.indice_stack, regPila);
	log_info(logger, "La posicion de la variable %c es %d", variable, posicion);
	return posicion;
}
t_valor_variable dereferenciar(t_puntero puntero) {
	/*
	 * me dan un puntero y devuelvo el valor (le pido a umc el valor de la var en ese puntero)
	 * me pasan directo el byte donde arranca la variable y tengo que transformar en pag y offset
	 */
	int valorVariable;
	int numeroPagina = puntero / tamanioPagina;
	int offset = puntero % tamanioPagina;
	t_posicion_memoria posicionMemoria;
	posicionMemoria.pagina = numeroPagina;
	posicionMemoria.offset = offset;
	posicionMemoria.size = TAM_VAR;
	if (enviarPedidosDePosicionMemoria(socketUMC, posicionMemoria, (void *) &valorVariable, tamanioPagina)) {
		enviarAbortarProgramaNucleo(socketNucleo);
		sigoEjecutando = 0;
		log_error(logger, "Direccion de memoria invalida");
		enviarPcb(socketNucleo, pcbRecibido);
	} else {
		log_info(logger, "Se derreferencio la dir %d y vale %d", puntero, valorVariable);
	}
	return valorVariable;
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	/*
	 * me pasan el byte donde arranco a guardar y el valor
	 * calculo pag offset y le pido a umc que lo guarde
	 */
	int numeroPagina = puntero / tamanioPagina;
	int offset = puntero % tamanioPagina;
	t_posicion_memoria posicionMemoria;
	posicionMemoria.pagina = numeroPagina;
	posicionMemoria.offset = offset;
	posicionMemoria.size = TAM_VAR;
	if (enviarAlmacenamientosDePosicionMemoria(socketUMC, posicionMemoria, (void *) &variable, tamanioPagina)) {
		enviarAbortarProgramaNucleo(socketNucleo);
		sigoEjecutando = 0;
		log_error(logger, "Stackoverflow en %d", puntero);
		enviarPcb(socketNucleo, pcbRecibido);
	}
}
int imprimir(t_valor_variable valor) {
	/*
	 * me pasan un valor y se lo paso a nucleo para que selo pase a ocnsola para imprimirlo
	 */
	//falta definir logger
	char* texto = string_itoa(valor);
	int largoTexto = strlen(texto);
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
	free(texto);
	return largoTexto;
}
int imprimirTexto(char* texto) {
	/*
	 * mismo que imprimir
	 */
	//falta definir logger
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
	int largoTexto = strlen(texto);
	return largoTexto;
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	/*
	 * le aviso al nucleo que un proceso quiere usar un dispositivo de e/s por tanto tiempo
	 * le aviso que bloquee el proceso
	 */
	huboEntradaSalida = 1;
	sigoEjecutando = 0;
	enviarEntradaSalida(socketNucleo, pcbRecibido, dispositivo, tiempo);

}

void parserWait(t_nombre_semaforo identificador_semaforo) {
	/*
	 * le dice a nucleo que el proceso ansisop quiere hacer wait de este semaforo
	 */
	enviarWait(socketNucleo, pcbRecibido.pid, identificador_semaforo);
}

void parserSignal(t_nombre_semaforo identificador_semaforo) {
	/*
	 * el prog ansisop hace un signal de este semaforo
	 */
	enviarSignal(socketNucleo, pcbRecibido.pid, identificador_semaforo);
}

void irAlLabel(t_nombre_etiqueta etiqueta) {
	/*
	 * cambio el pc a la primera instruccion de la etiqueta
	 */
	pcbRecibido.pc = metadata_buscar_etiqueta(etiqueta, pcbRecibido.indice_etiquetas.etiquetas, pcbRecibido.indice_etiquetas.largoTotalEtiquetas);
	huboSaltoLinea = 1;
}

void retornar(t_valor_variable retorno) {
	/*
	 * si stoy en main termino el proceso
	 * si no:
	 * guardo retorno en la var corresp al retorno (dir de retorno del reg pila)
	 * hago pop para sacar el reg del indice de stack, por ende vuevlo a la funcion anterior
	 */
	t_registro_pila* funcionOrigen = popPila(pcbRecibido.indice_stack);
	t_registro_pila* funcionDestino = popPila(pcbRecibido.indice_stack);
	if (funcionDestino == NULL) {
		enviarFinalizacionProgramaNucleo(socketNucleo);
		enviarPcb(socketNucleo, pcbRecibido);
		sigoEjecutando = 0;
	} else {
		if (enviarAlmacenamientosDePosicionMemoria(socketUMC, funcionOrigen->variable_retorno, &retorno,tamanioPagina)){
			enviarAbortarProgramaNucleo(socketNucleo);
			sigoEjecutando = 0;
			enviarPcb(socketNucleo, pcbRecibido);
		} else {
			pcbRecibido.pc = funcionOrigen->direccion_retorno;
			pushPila(pcbRecibido.indice_stack, funcionDestino);
			huboSaltoLinea = 1;

			int sizeLista = list_size(funcionOrigen->lista_argumentos), i;
			for (i = 0; i < sizeLista; i++) {
				free(list_remove(funcionOrigen->lista_argumentos, 0));
			}
			list_destroy(funcionOrigen->lista_argumentos);

			sizeLista = list_size(funcionOrigen->lista_variables);
			for (i = 0; i < sizeLista; i++) {
				free(list_remove(funcionOrigen->lista_variables, 0));
			}
			list_destroy(funcionOrigen->lista_variables);
			free(funcionOrigen);
		}
	}
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	/*
	 * creo un nuevo regPila en indice stack
	 */
	t_registro_pila * nuevoRegistroStack = malloc(sizeof(t_registro_pila));
	t_registro_pila * registroStackAnterior = popPila(pcbRecibido.indice_stack);
	nuevoRegistroStack->posicionUltimaVariable = registroStackAnterior->posicionUltimaVariable;
	nuevoRegistroStack->lista_argumentos = list_create();
	nuevoRegistroStack->lista_variables = list_create();
	nuevoRegistroStack->variable_retorno.pagina = donde_retornar / tamanioPagina;
	nuevoRegistroStack->variable_retorno.offset = donde_retornar % tamanioPagina;
	nuevoRegistroStack->variable_retorno.size = TAM_VAR;
	nuevoRegistroStack->direccion_retorno = pcbRecibido.pc + 1;
	pcbRecibido.pc = metadata_buscar_etiqueta(etiqueta, pcbRecibido.indice_etiquetas.etiquetas, pcbRecibido.indice_etiquetas.largoTotalEtiquetas);
	huboSaltoLinea = 1;
	pushPila(pcbRecibido.indice_stack, registroStackAnterior);
	pushPila(pcbRecibido.indice_stack, nuevoRegistroStack);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {
	/*
	 * le pido a nucleo el val de la var compartida
	 */
	t_valor_variable valorVariable;
	pedirCompartidaNucleo(socketNucleo, variable, &valorVariable);
	return valorVariable;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor) {
	/*
	 * le digo a nucleo que guarde el val de la var compartida
	 */
	asignarCompartidaNucleo(socketNucleo, variable, valor);
	return valor;
}

void finalizar() {

	enviarFinalizacionProgramaNucleo(socketNucleo);
	sigoEjecutando = 0;
	enviarPcb(socketNucleo, pcbRecibido);

}

