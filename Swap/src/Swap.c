/*
 ============================================================================
 Name        : Swap.c
 Author      : Dieta
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <structs.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "funciones.h"
#include "VariablesGlobales.h"

int main(int argc, char *argv[]) {

	t_config *config;

	if (argc != 2) {
		//	printf("Número incorrecto de parámetros\n");
		//	return -1;
		config = config_create(
				"/home/utnso/TP/tp-2016-1c--Dieta/Swap/Configuracion/config");
	} else {
		config = config_create(argv[1]);
	}

	int puertoServidor = config_get_int_value(config, "PUERTO_ESCUCHA");
	cantidadDeFrames = config_get_int_value(config, "CANTIDAD_PAGINAS");
	sizePagina = config_get_int_value(config, "TAMANIO_PAGINA");
	retardoAcceso = config_get_int_value(config, "RETARDO_ACCESO");
	retardoCompactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	char*nombre = config_get_string_value(config, "NOMBRE_SWAP");

	logger = log_create("Swap.log", "SWAP", 1, log_level_from_string("INFO"));

	int sizeTotal = sizePagina * cantidadDeFrames;
	char st[50];
	sprintf(st, "%d", sizeTotal);

	char comando[100] = "dd if=/dev/zero of=./";
	strcat(comando, nombre);
	strcat(comando, " bs=");
	strcat(comando, st);
	strcat(comando, " count=1");

	if (system(comando) != 0) {
		log_error(logger, "No se pudo crear el archivo.");
		return -1;
	}

	log_info(logger, "Se creó el archivo.");

	char pathArchivo[50] = "./";
	strcat(pathArchivo, nombre);

	int fd = open(pathArchivo, O_RDWR);

	if (fd == -1) {
		printf("No se pudo abrir el archivo");
		return -1;
	};

	char*archivoSwap = mmap((caddr_t)  0, sizeTotal, PROT_READ | PROT_WRITE,
	MAP_PRIVATE, fd, 0);

	int i;
	bitMap = malloc(sizeof(int) * cantidadDeFrames);
	for (i = 0; i < cantidadDeFrames; i++) {
		bitMap[i] = 0;
	}

	listaProcesos = list_create();
//---------------------------------------------------------------------------
	int servidorSwap;
	if (crearSocket(&servidorSwap)) {
		printf("Error creando socket");
		log_error(logger, "Se produjo un error al crear el socket");
		return 1;
	}

	log_info(logger, "Se estableció el socket de escucha correctamente");

	if (escucharEn(servidorSwap, puertoServidor)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error al tratar de conectar con el socket");
		return 1;
	}

//-------------------------------------------------------------------------
	struct sockaddr_in direccionCliente;

	int cliente = aceptarConexion(servidorSwap, &direccionCliente);

//-------------------------------------------------------------------------
	int idRecibido;
	idRecibido = iniciarHandshake(cliente, IDSWAP);

	if (idRecibido != IDUMC) {
		printf("ERROR NO SE CONECTO A UMC");
		log_error(logger, "Se produjo un error al conectarse a UMC");
		return -1;
	}

	log_info(logger, "Se conectó UMC");

	while (1) {
		int header = recibirHeader(cliente);

		switch (header) {
		case 0:
			log_error(logger, "Se desconectó UMC. Abortando Swap");
			abort();
			break;
		case inicializarProgramaSwap:
			iniciarProgramaAnsisop(cliente, archivoSwap);
			break;
		case guardarPaginasEnSwap:
			guardarPaginas(cliente, archivoSwap);
			break;
		case pedidoPaginaASwap:
			enviarPaginas(cliente, archivoSwap);
			break;
		case finalizacionPrograma:
			finalizarProgramaAnsisop(cliente);
			break;
		}

	}

//---------------------------------------------------------------------------

	config_destroy(config);
	close(fd);
	log_destroy(logger);
	return 0;
}

