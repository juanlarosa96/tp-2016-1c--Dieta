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
	size_frames = config_get_int_value(config, "MARCO_SIZE");
	entradasTLB = config_get_int_value(config, "ENTRADAS_TLB");
	retardo = config_get_int_value(config, "RETARDO");
	framesPorProceso = config_get_int_value(config, "MARCO_X_PROC");
	algoritmo = config_get_string_value(config, "ALGORITMO");

			//Reservo Memoria
	tamanioMemoria = (cant_frames) * (size_frames);
	memoriaPrincipal = malloc(tamanioMemoria);
	memset(memoriaPrincipal, 0, sizeof(tamanioMemoria));

	//Inicializo listas compartidas
	listaFrames = list_create();
	listaProcesos = list_create();
	TLB = list_create();

	//Inicializo mutex
	pthread_mutex_init(&mutexFrames, NULL);
	pthread_mutex_init(&mutexProcesos, NULL);
	pthread_mutex_init(&mutexSwap, NULL);
	pthread_mutex_init(&mutexTLB, NULL);
	pthread_mutex_init(&mutexContadorMemoria, NULL);
	pthread_mutex_init(&mutexMemoriaPrincipal, NULL);
	pthread_mutex_init(&mutexRetardo, NULL);

	//Inicializo variables globales
	texto = "info";
	accesoMemoria = 0;

	//Inicializo log para UMC
	logger = log_create("UMC.log", "UMC", 1, log_level_from_string("INFO"));

	//Hilo para Consola de UMC
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void *) consolaUMC, NULL);

	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char * ip_swap = config_get_string_value(config, "IP_SWAP");

	/*---------SOCKET CLIENTE DE SWAP------------*/

	if (crearSocket(&socketSwap)) {
		printf("Error creando socket\n");
		log_error(logger, "Se produjo un error creando el socket cliente de Swap",
				texto);
		return 1;
	}

	if (conectarA(socketSwap, ip_swap, puerto_swap)) {
		printf("Error al conectar\n");
		log_error(logger, "Se produjo un error conectandose a Swap", texto);
		return 1;
	}

	if (responderHandshake(socketSwap, IDUMC, IDSWAP)) {
		log_error(logger, "Error en el handshake con SWAP", texto);
		return 1;
	}

	log_info(logger, "Se conectó correctamente a SWAP", texto);

	/*---------SOCKET SERVIDOR DE NUCLEO Y CPUs------------*/

	int servidorUMC;
	if (crearSocket(&servidorUMC)) {
		log_error(logger, "Error creando socket servidor para Nucleo y CPUs",
				texto);
		return 1;
	}
	if (escucharEn(servidorUMC, puerto_servidor)) {
		printf("Error al conectar"); //?
		//log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	log_info(logger, "Escuchando nuevas conexiones");

	/*-----------CONEXION CON NUCLEO-----------------*/

	//PRIMERO SE TIENE QUE CONECTAR NUCLEO ANTES QUE LAS CPUS
	int nuevaConexion;
	struct sockaddr_in direccionCliente;

	nuevaConexion = aceptarConexion(servidorUMC, &direccionCliente);
	int idRecibido = iniciarHandshake(nuevaConexion, IDUMC);

	while (idRecibido != IDNUCLEO) {
		log_error(logger,
				"Se esperaba que se conectara Nucleo. Conexion desconocida",
				texto);
		close(nuevaConexion);

		nuevaConexion = aceptarConexion(servidorUMC, &direccionCliente);
		idRecibido = iniciarHandshake(nuevaConexion, IDUMC);
	}

	log_info(logger, "Se estableció la conexión con Núcleo", texto);


	enviarTamanioPagina(nuevaConexion, size_frames);

	//Hilo para manejar las solicitudes de Nucleo
	int * socketNucleoPuntero = malloc(sizeof(int));
	*socketNucleoPuntero = nuevaConexion;

	pthread_attr_t attr;
	pthread_t hiloNucleo;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloNucleo, &attr, (void *) procesarOperacionesNucleo, socketNucleoPuntero);
	pthread_attr_destroy(&attr);

	/*-----------CONEXION CON CPUs-----------*/

	int conexionCPU;
	struct sockaddr_in direccionCPU;
	int id;

	while (1) {
		conexionCPU = aceptarConexion(servidorUMC, &direccionCPU);
		id = iniciarHandshake(conexionCPU, IDUMC);
		if (id == IDCPU) {
			enviarTamanioPagina(conexionCPU, size_frames);

			int * socketCPU = malloc(sizeof(int));
			*socketCPU = conexionCPU;

			pthread_attr_t attr;
			pthread_t hiloCPU;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_create(&hiloCPU, &attr,
					(void *) procesarSolicitudOperacionCPU, socketCPU);
			pthread_attr_destroy(&attr);

			log_info(logger, "Nuevo CPU conectado");
		} else {
			log_error(logger, "Se esperaba un CPU. Conexion inesperada");
			close(conexionCPU);
		}

	}

	pthread_join(hiloConsola, NULL);

	config_destroy(config);
	log_destroy(logger);

	return EXIT_SUCCESS;

}
