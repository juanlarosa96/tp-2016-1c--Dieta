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
	posicionVariable->size = 4;
	regPila->posicionUltimaVariable += 4;

	int aux = variable - '0';
	if(aux>= 0 && aux <=9){
		list_add(regPila->lista_argumentos,posicionVariable);
	} else{
		t_identificadorConPosicionMemoria * nuevaVariable = malloc(sizeof (t_identificadorConPosicionMemoria));
		nuevaVariable->identificador = variable;
		nuevaVariable->posicionDeVariable = *posicionVariable;
		free(posicionVariable);
	}

	pushPila(pcbRecibido.indice_stack,regPila);
	return regPila->posicionUltimaVariable - 4;

}
t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtengo posición variable\n");
	return variable;
}
t_valor_variable dereferenciar(t_puntero puntero) {
	int valorVariable;
	int numeroPagina = puntero/tamanioPagina;
	int offset = puntero%tamanioPagina;
	t_posicion_memoria posicionMemoria;
	posicionMemoria.pagina = numeroPagina;
	posicionMemoria.offset = offset;
	posicionMemoria.size = 4;
	enviarPedidosDePosicionMemoria(socketUMC, posicionMemoria, (void *) & valorVariable, tamanioPagina);
	if(recibirHeader(socketUMC) == pedidoMemoriaFallo){
		enviarAbortarProgramaNucleo(socketNucleo);
		sigoEjecutando = 0;
	}
	return valorVariable;
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	int numeroPagina = puntero/tamanioPagina;
	int offset = puntero%tamanioPagina;
	t_posicion_memoria posicionMemoria;
	posicionMemoria.pagina = numeroPagina;
	posicionMemoria.offset = offset;
	posicionMemoria.size = 4;
	enviarAlmacenamientosDePosicionMemoria(socketUMC, posicionMemoria, (void *) &variable, tamanioPagina);
	if(recibirHeader(socketUMC) == pedidoMemoriaFallo){
		enviarAbortarProgramaNucleo(socketNucleo);
		sigoEjecutando = 0;
	}
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
	huboEntradaSalida = 1;
	sigoEjecutando = 0;
	enviarEntradaSalida(socketNucleo, pcbRecibido, dispositivo, tiempo);
}

void wait(t_nombre_semaforo identificador_semaforo) {
	enviarWait(socketNucleo, pcbRecibido.pid, identificador_semaforo);
}

void signal(t_nombre_semaforo identificador_semaforo) {
	enviarSignal(socketNucleo, pcbRecibido.pid, identificador_semaforo);
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	pcbRecibido.pc = metadata_buscar_etiqueta(etiqueta,pcbRecibido.indice_etiquetas.etiquetas,pcbRecibido.indice_etiquetas.largoTotalEtiquetas);
	huboSaltoLinea = 1;
}

void retornar(t_valor_variable retorno){

	t_registro_pila* funcionOrigen = popPila(pcbRecibido.indice_stack);
	t_registro_pila* funcionDestino = popPila(pcbRecibido.indice_stack);
	if(funcionDestino == NULL){
		enviarFinalizacionProgramaNucleo(socketNucleo);
		sigoEjecutando = 0;
	}else{
		enviarPedidoAlmacenarBytes(socketUMC,funcionOrigen->variable_retorno.pagina,funcionOrigen->variable_retorno.offset,4,&retorno);
		if(recibirHeader(socketUMC) == pedidoMemoriaFallo){
			enviarAbortarProgramaNucleo(socketNucleo);
			sigoEjecutando = 0;
		}else{
			pcbRecibido.pc = funcionOrigen->direccion_retorno;
			pushPila(pcbRecibido.indice_stack,funcionDestino);
			huboSaltoLinea = 1;

			int sizeLista = list_size(funcionOrigen->lista_argumentos),i;
			for(i = 0; i < sizeLista; i++){
				free(list_remove(funcionOrigen->lista_argumentos,0));
				}
			list_destroy(funcionOrigen->lista_argumentos);

			sizeLista = list_size(funcionOrigen->lista_variables);
			for(i = 0; i < sizeLista; i++){
				free(list_remove(funcionOrigen->lista_variables,0));
			}
			list_destroy(funcionOrigen->lista_variables);
			free(funcionOrigen);
		}
	}
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

	t_registro_pila * nuevoRegistroStack = malloc(sizeof(t_registro_pila));
	t_registro_pila * registroStackAnterior = popPila(pcbRecibido.indice_stack);
	nuevoRegistroStack->posicionUltimaVariable = registroStackAnterior->posicionUltimaVariable;
	list_create(nuevoRegistroStack->lista_argumentos);
	list_create(nuevoRegistroStack->lista_variables);
	nuevoRegistroStack->variable_retorno.pagina = donde_retornar / tamanioPagina;
	nuevoRegistroStack->variable_retorno.offset = donde_retornar % tamanioPagina;
	nuevoRegistroStack->variable_retorno.size = 4;
	nuevoRegistroStack->direccion_retorno = metadata_buscar_etiqueta(etiqueta,pcbRecibido.indice_etiquetas.etiquetas,pcbRecibido.indice_etiquetas.largoTotalEtiquetas);

	pushPila(pcbRecibido.indice_stack,registroStackAnterior);
	pushPila(pcbRecibido.indice_stack,nuevoRegistroStack);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valorVariable;
	pedirCompartidaNucleo(socketNucleo, variable, &valorVariable);
	return valorVariable;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	asignarCompartidaNucleo(socketNucleo, variable, valor);
	return valor;
}
