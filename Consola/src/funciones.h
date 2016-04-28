/*
 * funciones.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_


#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <parser/parser.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <commons/string.h>
#include <stdint.h>


t_puntero definirVariable(t_nombre_variable variable);
t_puntero obtenerPosicionVariable(t_nombre_variable variable);
t_valor_variable dereferenciar(t_puntero puntero);
void asignar(t_puntero puntero, t_valor_variable variable);
void imprimir(t_valor_variable valor);
void imprimirTexto(char* texto);

#endif /* FUNCIONES_H_ */
