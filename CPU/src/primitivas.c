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

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	//implementar logger
	enviarEntradaSalida(socketNucleo, pcbRecibido.pid, dispositivo, tiempo);
}

