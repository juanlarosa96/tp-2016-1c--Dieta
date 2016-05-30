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

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>

#include <commons/config.h>
#include <string.h>

#include "funcionesUMC.h"


int main(int argc, char *argv[]) {

	t_config* config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config.cfg");
	} else {

		config = config_create(argv[1]);
	}

	int cant_frames = config_get_int_value(config, "MARCOS");
	int size_frames = config_get_int_value(config, "MARCO_SIZE");

	//Reservo Memoria
	int memoriaDisponible = (cant_frames) * (size_frames);
	void * memoria = malloc(memoriaDisponible);
	memset(memoria, 0, sizeof(memoriaDisponible));

	//Inicializo variables globales
	listaFrames = list_create();
	listaProcesos = list_create();
	texto = "info";
	pthread_mutex_init(&mutexFrames, NULL);
	pthread_mutex_init(&mutexProcesos, NULL);

	//Log para UMC
	logger = log_create("UMC.log", "UMC", 1, log_level_from_string("INFO"));

	//Hilo para Consola de UMC



	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char * ip_swap = config_get_string_value(config, "IP_SWAP");

	/*---------SOCKET CLIENTE DE SWAP------------*/

	int clienteSwap; //esto tiene que ser variable global?
	if (crearSocket(&clienteSwap)) {
		printf("Error creando socket\n");
		log_error(logger, "Se produjo un error creando el socket de UMC", texto);
		return 1;
	}

	if (conectarA(clienteSwap, ip_swap, puerto_swap)) {
		printf("Error al conectar\n");
		log_error(logger, "Se produjo un error conectandose a Swap", texto);
		return 1;
	}


	if (responderHandshake(clienteSwap, IDUMC, IDSWAP)) {
		log_error(logger, "Error en el handshake con SWAP", texto);
		return 1;
	}

	log_info(logger, "Se conectó correctamente a SWAP", texto);



	/*---------SOCKET SERVIDOR DE NUCLEO Y CPUs------------*/

	int servidorUMC;
	if (crearSocket(&servidorUMC)) {
		log_error(logger, "Error creando socket servidor para Nucleo y CPUs", texto);
		return 1;
	}
	if (escucharEn(servidorUMC, puerto_servidor)) {
		printf("Error al conectar"); //?
		//log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	printf("Escuchando\n");

	/*-----------CONEXION CON NUCLEO-----------------*/

	//PRIMERO SE TIENE QUE CONECTAR NUCLEO ANTES QUE LAS CPUS
	int nuevaConexion;
	struct sockaddr_in direccionCliente;

	nuevaConexion = aceptarConexion(servidorUMC, &direccionCliente);
	int idRecibido = iniciarHandshake(nuevaConexion, IDUMC);

	while(idRecibido != IDNUCLEO){
		log_error(logger, "Se esperaba que se conectara Nucleo. Conexion desconocida", texto);
		close(nuevaConexion);

		nuevaConexion = aceptarConexion(servidorUMC, &direccionCliente);
		idRecibido = iniciarHandshake(nuevaConexion, IDUMC);
	}

	log_info(logger, "Se estableció la conexión con Núcleo",texto);

	enviarTamanioPagina(nuevaConexion, size_frames);
	//Hilo para manejar las solicitudes de Nucleo
	pthread_attr_t attr;
	pthread_t hiloCPU;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloCPU,&attr,(void *) &procesarOperacionesNucleo, (void *) nuevaConexion);
	pthread_attr_destroy(&attr);



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
			pthread_attr_destroy(&attr);

			log_info(logger, "Nuevo CPU conectado", texto);
		}
		else {
			log_error(logger, "Se esperaba un CPU. Conexion inesperada", texto);
			close(nuevaConexion);
		}

	}

	//pthread_join() del hilo de consola

	config_destroy(config);

	return EXIT_SUCCESS;

}
