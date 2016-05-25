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

#include <sockets.h>
#include <protocolo.h>


int main(int argc, char *argv[]) {

	t_config* config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}

	int cant_frames = config_get_int_value(config,"MARCOS");
	int size_frames = config_get_int_value(config, "MARCOS_SIZE");

	//Reservo Memoria
	int memoriaDisponible = cant_frames * size_frames;
	int * memoria = malloc(memoriaDisponible);
	memset(memoria,0,sizeof(memoriaDisponible));

	t_list * listaFrames = list_create();
	t_list * listaProcesos = list_create();


	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char* ip_swap = config_get_string_value(config, "IP_SWAP");


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

	
	/*----------SELECT----------------*/

	fd_set setSockets;
	fd_set setAuxiliar;  
	int fdmax;	    // maximum file descriptor number
	int listener = servidorUMC; 
	int nuevaConexion;   // newly accept()ed socket descriptor

	FD_ZERO(&setSockets);
	FD_ZERO(&setAuxiliar);

	FD_SET(listener, &setSockets);

	//fdmax = (servidorUMC > socketCliente) ? servidorUMC : socketCliente;

	fdmax = listener;

	struct sockaddr_in direccionCliente;


	char buf[256];    // buffer for client data
	int nbytesRecibidos;

	//struct addrinfo hints, *ai, *p;

	int i = 0;
	while (1) {
		setAuxiliar = setSockets;
		if (select(fdmax + 1, &setAuxiliar, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &setAuxiliar)) {
				if (i == listener) {
					nuevaConexion = aceptarConexion(i, &direccionCliente);
					int idRecibido = iniciarHandshake(nuevaConexion, IDUMC);

					switch (idRecibido) {
					case 0:
						//log_info(logger, "Se desconecto el socket", texto); NO ESTOY SEGURA BIEN EN ESTE CASO
						close(nuevaConexion);
						break;
					case IDCPU:
						FD_SET(nuevaConexion, &setSockets);
						enviarTamanioPagina(nuevaConexion, size_frames);
						pthread_t nuevoHiloCPU;
						//pthread_create(&nuevoHiloCPU, NULL,(void *) &procesarSolicitudOperacion, (void *) &i);
  					     //el hilo va a servir para las solicitudes de operaciones

						//log_info(logger, "Nuevo CPU conectado", texto);
						break;
					case IDNUCLEO:
						FD_SET(nuevaConexion, &setSockets);
						enviarTamanioPagina(nuevaConexion, size_frames);

						break;
					default:
						close(nuevaConexion);
						//log_error(logger, "Error en el handshake. Conexion inesperada", texto);
						break;
					}

					if(nuevaConexion > fdmax){
						fdmax = nuevaConexion;
					}

				} else {
					//fd clear
					//manejar los mensajes de nucleo a umc

					printf("Holis\n");
				}
			}
		}
	}



	config_destroy(config);

	return EXIT_SUCCESS;

}
