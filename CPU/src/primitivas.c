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
	printf("Dereferenciar\n");
	return puntero;
}
void asignar(t_puntero puntero, t_valor_variable variable) {
	int numeroPagina = puntero/tamanioPagina;
	int offset = puntero%tamanioPagina;
	enviarPedidoAlmacenarBytes(socketUMC,numeroPagina,offset,4,&variable);
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
}

