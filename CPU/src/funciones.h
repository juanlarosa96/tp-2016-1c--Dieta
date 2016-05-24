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
#include <commons/config.h>
#include <structs.h>


t_posicion_memoria obtenerPosicionPagina(int tamanioPagina, t_pcb unPcb);
char* recibirLineaAnsisop(int socketUMC, t_posicion_memoria posicionPagina, char* holis);




#endif /* FUNCIONES_H_ */
