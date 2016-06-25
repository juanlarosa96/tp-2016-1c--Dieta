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
#include <unistd.h>
#include <protocolo.h>
#include <sockets.h>
#include <pthread.h>

t_log* logger;
void interpreteComandos(int * socketNucleo);

#endif /* FUNCIONES_H_ */
