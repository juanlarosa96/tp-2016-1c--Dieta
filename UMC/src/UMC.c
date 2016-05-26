/*
 ============================================================================
 Name        : UMC.c
 Author      : Dieta
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "funcionesUMC.h"

int main(int argc, char *argv[]) {

	t_config* config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}

	int cant_frames = config_get_int_value(config, "MARCOS");
	int size_frames = config_get_int_value(config, "MARCOS_SIZE");

	//Reservo Memoria
	int memoriaDisponible = cant_frames * size_frames;
	int * memoria = malloc(memoriaDisponible);
	memset(memoria, 0, sizeof(memoriaDisponible));

	t_list * listaFrames = list_create();
	t_list * listaProcesos = list_create();

	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char* ip_swap = config_get_string_value(config, "IP_SWAP");

	/*---------SOCKET CLIENTE DE SWAP------------*/

	int clienteSwap;
	if (crearSocket(&clienteSwap)) {
		printf("Error creando socket\n");
		//log_error(logger, "Se produjo un error creando el socket de UMC", texto);
		return 1;
	}
	if (conectarA(clienteSwap, ip_swap, puerto_swap)) {
		printf("Error al conectar\n");
		//log_error(logger, "Se produjo un error conectandose a Swap", texto);
		return 1;
	}

	if (responderHandshake(clienteSwap, IDUMC, IDSWAP)) {
		//log_error(logger, "Error en el handshake", texto);
		return 1;

	}

	/*---------SOCKET SERVIDOR DE NUCLEO Y CPUs------------*/

	int servidorUMC;
	if (crearSocket(&servidorUMC)) {
		printf("Error creando socket");
		return 1;
	}
	if (escucharEn(servidorUMC, puerto_servidor)) {
		printf("Error al conectar");
		//log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	//log_info(logger, "Se estableció correctamente el socket servidor", texto);
	printf("Escuchando\n");

	/*-----------CONEXION CON NUCLEO-----------------*/

	//NOTA: PRIMERO SE TIENE QUE CONECTAR NUCLEO, SI NO SE ROMPE TODO
	int nuevaConexion;
	struct sockaddr_in direccionCliente;
	nuevaConexion = aceptarConexion(servidorUMC, &direccionCliente);
	int idRecibido = iniciarHandshake(nuevaConexion, IDUMC);

	if (idRecibido == IDNUCLEO) {
		enviarTamanioPagina(nuevaConexion, size_frames);
		//crear hilo para nucleo
	}

	/*-----------CONEXION CON CPUs-----------*/

	int conexionCPU;
	struct sockaddr_in direccionCPU;
	int id;

	while (1) {
		conexionCPU = aceptarConexion(servidorUMC, &direccionCPU);
		id = iniciarHandshake(conexionCPU, IDUMC);
		if(id == IDCPU){
			enviarTamanioPagina(conexionCPU, size_frames);

			pthread_attr_t attr;
			pthread_t hiloCPU;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			pthread_create(&hiloCPU,&attr,(void *) &procesarSolicitudOperacionCPU, (void *) conexionCPU);

			//log_info(logger, "Nuevo CPU conectado", texto);
		}
		else {
			close(nuevaConexion);
			//log_error(logger, "Error en el handshake. Conexion inesperada", texto);
		}

	}

	config_destroy(config);

	return EXIT_SUCCESS;

}
