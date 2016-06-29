/*
 * primitivas.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable variable) {
	if (sigoEjecutando){
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
		log_info(logger, "Se definio la variable %c en la posicion ", variable, regPila->posicionUltimaVariable - TAM_VAR);
		return regPila->posicionUltimaVariable - TAM_VAR;
	} else {
		return 0;
	}
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	/*
	 * me pasan un identif y me fijo en el stack de cpu cual es la pos
	 */
	if (sigoEjecutando){
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
	} else {
		return 0;
	}
}
t_valor_variable dereferenciar(t_puntero puntero) {
	/*
	 * me dan un puntero y devuelvo el valor (le pido a umc el valor de la var en ese puntero)
	 * me pasan directo el byte donde arranca la variable y tengo que transformar en pag y offset
	 */
	if (sigoEjecutando){
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
	} else {
		return 0;
	}
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	/*
	 * me pasan el byte donde arranco a guardar y el valor
	 * calculo pag offset y le pido a umc que lo guarde
	 */
	if (sigoEjecutando){
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
		} else {
			log_info(logger, "Asigno valor %d en la posicion %d", variable, puntero);
		}
	}
}
int imprimir(t_valor_variable valor) {
	/*
	 * me pasan un valor y se lo paso a nucleo para que selo pase a ocnsola para imprimirlo
	 */
	if (sigoEjecutando){
		char* texto = string_itoa(valor);
		int largoTexto = strlen(texto);
		log_info(logger, "Envio a imprimir %d", valor);
		enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
		free(texto);
		return largoTexto;
	} else {
		return 0;
	}
}
int imprimirTexto(char* texto) {
	/*
	 * mismo que imprimir
	 */
	if (sigoEjecutando){
		log_info(logger, "Envio a imprimir %s", texto);
		enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
		int largoTexto = strlen(texto);
		return largoTexto;
	} else {
		return 0;
	}
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	/*
	 * le aviso al nucleo que un proceso quiere usar un dispositivo de e/s por tanto tiempo
	 * le aviso que bloquee el proceso
	 */
	if (sigoEjecutando){
		huboEntradaSalida = 1;
		sigoEjecutando = 0;
		log_info(logger, "Envio io al dispositivo %s por %d unidades de tiempo", dispositivo, tiempo);
		pcbRecibido.pc++;
		enviarEntradaSalida(socketNucleo, pcbRecibido, dispositivo, tiempo);
	}
}

void parserWait(t_nombre_semaforo identificador_semaforo) {
	/*
	 * le dice a nucleo que el proceso ansisop quiere hacer wait de este semaforo
	 */
	if (sigoEjecutando){
		identificador_semaforo[strlen(identificador_semaforo) -1] = '\0';
		log_info(logger, "Envio wait semaforo %c", identificador_semaforo);
		enviarWait(socketNucleo, pcbRecibido.pid, identificador_semaforo);
		if(recibirHeader(socketNucleo) == headerBloquear){
			sigoEjecutando = 0;
			log_info(logger, "Bloqueado por wait en el semaforo %c", identificador_semaforo);
			pcbRecibido.pc++;
			enviarPcb(socketNucleo, pcbRecibido);
		}
	}
}

void parserSignal(t_nombre_semaforo identificador_semaforo) {
	/*
	 * el prog ansisop hace un signal de este semaforo
	 */
	if (sigoEjecutando){
		identificador_semaforo[strlen(identificador_semaforo) -1] = '\0';
		log_info(logger, "Envio signal semaforo %c", identificador_semaforo);
		enviarSignal(socketNucleo, pcbRecibido.pid, identificador_semaforo);
	}
}

void irAlLabel(t_nombre_etiqueta etiqueta) {
	/*
	 * cambio el pc a la primera instruccion de la etiqueta
	 */
	if (sigoEjecutando){
		pcbRecibido.pc = metadata_buscar_etiqueta(etiqueta, pcbRecibido.indice_etiquetas.etiquetas, pcbRecibido.indice_etiquetas.largoTotalEtiquetas);
		huboSaltoLinea = 1;
		log_info(logger, "Hubo salto de linea a la etiqueta %s", etiqueta);
	}
}

void retornar(t_valor_variable retorno) {
	/*
	 * si stoy en main termino el proceso
	 * si no:
	 * guardo retorno en la var corresp al retorno (dir de retorno del reg pila)
	 * hago pop para sacar el reg del indice de stack, por ende vuevlo a la funcion anterior
	 */
	if (sigoEjecutando){
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
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	/*
	 * creo un nuevo regPila en indice stack
	 */
	if (sigoEjecutando){
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
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {
	/*
	 * le pido a nucleo el val de la var compartida
	 */
	if (sigoEjecutando){
		t_valor_variable valorVariable;
		pedirCompartidaNucleo(socketNucleo, variable, &valorVariable);
		log_info(logger, "Pido valor variable compartida %s y es %d", variable, valorVariable);
		return valorVariable;
	} else {
		return 0;
	}
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor) {
	/*
	 * le digo a nucleo que guarde el val de la var compartida
	 */
	if (sigoEjecutando){
		asignarCompartidaNucleo(socketNucleo, variable, valor);
		log_info(logger, "Asigno valor %d a la variable compartida %s",valor, variable);
		return valor;
	}
}

void finalizar() {
	enviarFinalizacionProgramaNucleo(socketNucleo);
	sigoEjecutando = 0;
	log_info(logger, "Finalizo proceso con pid %d", pcbRecibido.pid);
	enviarPcb(socketNucleo, pcbRecibido);
}
