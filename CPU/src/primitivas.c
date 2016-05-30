/*
 * primitivas.c
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable variable) {
	printf("Defino variable\n");
	return variable;
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
void imprimir(t_valor_variable valor) {
	//falta definir logger
	char* texto = string_itoa(valor);
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
	free(texto);
}
void imprimirTexto(char* texto) {
	//falta definir logger
	enviarValorAImprimir(socketNucleo, pcbRecibido.pid, texto);
}
