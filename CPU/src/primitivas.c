/*
 * primitivas.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable variable) {
	t_list* stack = pcbRecibido.indice_stack;
	t_identificadorConPosicionMemoria * posicionVariable = malloc(sizeof(t_identificadorConPosicionMemoria));
	t_registro_pila * nodoPila;
	posicionVariable->identificador = variable;

	if (list_is_empty(stack)) {
		nodoPila = malloc(sizeof(t_registro_pila));
		nodoPila->lista_argumentos = list_create();
		nodoPila->lista_variables = list_create();

		posicionVariable->posicionDeVariable.pagina = pcbRecibido.paginas_codigo;
		posicionVariable->posicionDeVariable.offset = 0;
		posicionVariable->posicionDeVariable.size = sizeof(uint32_t);

		list_add(nodoPila->lista_variables, (void *) posicionVariable);
		pushPila(stack, nodoPila);

	} else {
		int sizePila = list_size(stack);
		if (sizePila == 1) {
			nodoPila = (t_registro_pila *) list_get(stack, sizePila - 1);
			int sizeLista = list_size(nodoPila->lista_variables);
			t_identificadorConPosicionMemoria * posicionVariableAnterior = list_get(nodoPila->lista_variables, sizeLista - 1);

			if (posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size > tamanioPagina) {

				posicionVariable->posicionDeVariable.pagina = posicionVariableAnterior->posicionDeVariable.pagina
						+ (posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size) / tamanioPagina;

				int offset = posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size;
				while (offset > tamanioPagina) {
					offset = offset - tamanioPagina;
				}
				posicionVariable->posicionDeVariable.offset = offset;
			} else {
				posicionVariable->posicionDeVariable.pagina = posicionVariableAnterior->posicionDeVariable.pagina;
				posicionVariable->posicionDeVariable.offset = posicionVariableAnterior->posicionDeVariable.offset
						+ posicionVariableAnterior->posicionDeVariable.size;
			}
			list_add(nodoPila->lista_variables, (void *) posicionVariable);
		} else {
			nodoPila = (t_registro_pila *) list_get(stack, sizePila - 1);
			int tamanioPila = list_size(stack);
			t_registro_pila * nodoPilaAnterior;
			for (tamanioPila; tamanioPila == 0; tamanioPila--) {
				nodoPilaAnterior = (t_registro_pila *) list_get(stack, tamanioPila - 1);
				if (!(list_is_empty(nodoPilaAnterior->lista_variables))) {

					int sizeLista = list_size(nodoPilaAnterior->lista_variables); //Si la lista del elemento anterior de la pila esta vacia
					t_identificadorConPosicionMemoria * posicionVariableAnterior = list_get(nodoPilaAnterior->lista_variables, sizeLista - 1);

					if (posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size > tamanioPagina) {

						posicionVariable->posicionDeVariable.pagina = posicionVariableAnterior->posicionDeVariable.pagina
								+ (posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size)
										/ tamanioPagina;

						int offset = posicionVariableAnterior->posicionDeVariable.offset + posicionVariableAnterior->posicionDeVariable.size;
						while (offset > tamanioPagina) {
							offset = offset - tamanioPagina;
						}
						posicionVariable->posicionDeVariable.offset = offset;
					} else {
						posicionVariable->posicionDeVariable.pagina = posicionVariableAnterior->posicionDeVariable.pagina;
						posicionVariable->posicionDeVariable.offset = posicionVariableAnterior->posicionDeVariable.offset
								+ posicionVariableAnterior->posicionDeVariable.size;
					}

				}
				list_add(nodoPila->lista_variables, (void *) posicionVariable);
				break;
			}
			if (tamanioPila == 0){
				posicionVariable->posicionDeVariable.pagina = pcbRecibido.paginas_codigo;
				posicionVariable->posicionDeVariable.offset = 0;
				posicionVariable->posicionDeVariable.size = sizeof(uint32_t);

				list_add(nodoPila->lista_variables, (void *) posicionVariable);

			}

		}

	}
	printf("Defino variable\n");
	return posicionVariable->posicionDeVariable.pagina * tamanioPagina + posicionVariable->posicionDeVariable.offset;
}
t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtengo posición variable\n");
	return variable;
}
t_valor_variable dereferenciar(t_puntero puntero) {
	printf("Dereferenciar\n");
	return puntero;
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignar\n");
}
int imprimir(t_valor_variable valor) {
	//falta definir logger
	char* texto = string_itoa(valor);
	int largoTexto = strlen(texto);
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
	free(texto);
	return largoTexto;
}
int imprimirTexto(char* texto) {
	//falta definir logger
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
	int largoTexto = strlen(texto);
	return largoTexto;
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	enviarEntradaSalida(socketNucleo, pcbRecibido.pid, dispositivo, tiempo);

}

void wait(t_nombre_semaforo identificador_semaforo) {
	enviarWait(socketNucleo, pcbRecibido.pid, identificador_semaforo);

}

void signal(t_nombre_semaforo identificador_semaforo) {
	enviarSignal(socketNucleo, pcbRecibido.pid, identificador_semaforo);

}
