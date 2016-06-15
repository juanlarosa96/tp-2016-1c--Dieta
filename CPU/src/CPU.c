#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "funciones.h"
#include "commons/log.h"
#include <structs.h>
#include <parser/metadata_program.h>
#include "primitivas.h"
#include "variables_globales.h"

AnSISOP_funciones functions = { .AnSISOP_definirVariable = definirVariable, .AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar = dereferenciar, .AnSISOP_asignar = asignar, .AnSISOP_imprimir = imprimir, .AnSISOP_imprimirTexto = imprimirTexto,
		.AnSISOP_retornar = retornar, .AnSISOP_llamarConRetorno = llamarConRetorno, .AnSISOP_finalizar = finalizar,

};

AnSISOP_kernel kernel_functions = { };

int main(int argc, char *argv[]) {
	//Recibe el archivo de config por parametro
	/*if (argc != 2) {
	 printf("Número incorrecto de parámetros\n");
	 return -1;
	 }

	 t_config* config = config_create(argv[1]);*/

	t_config* config;
	if (argc != 2) {
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}
	int id_cpu = getpid();
	char* nombreLogger = string_from_format("CPU ID %d.log",id_cpu);
	logger = log_create(nombreLogger, "CPU", 1, log_level_from_string("INFO"));
	char *texto;
	texto = "info";

	int puerto_umc = config_get_int_value(config, "PUERTO_UMC");
	int puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");
	char* ip_umc = config_get_string_value(config, "IP_UMC");
	char* ip_nucleo = config_get_string_value(config, "IP_NUCLEO");

	//creo socket nucleo
	crearSocket(&socketNucleo);

	//me intento conectar
	if (conectarA(socketNucleo, ip_nucleo, puerto_nucleo)) {
		perror("No se pudo conectar");
		log_error(logger, "No se pudo conectar al Nucleo", texto);
		return 1;
	}
	//handshake
	if (responderHandshake(socketNucleo, IDCPU, IDNUCLEO)) {
		log_error(logger, "Error en el handshake", texto);
		return 1;
	}

	//creo socket umc
	crearSocket(&socketUMC);

	//me intento conectar
	if (conectarA(socketUMC, ip_umc, puerto_umc)) {
		perror("No se pudo conectar");
		log_error(logger, "No se pudo conectar a la UMC", texto);
		return 1;
	}
	//handshake
	if (responderHandshake(socketUMC, IDCPU, IDUMC)) {
		log_error(logger, "Error en el handshake", texto);
		return 1;
	}
	if (recibirHeader(socketUMC) == tamanioDePagina) {
		tamanioPagina = recibirTamanioPagina(socketUMC);
	} else {

		log_error(logger, "Error recibiendo el tamaño de pagina", texto);
		return 1;
	}
	signal(SIGUSR1, manejadorSIGUSR1);
	signalApagado = 0;
	int header;
	int quantumTotal = 0, quantumRetardo = 0;

	while (!signalApagado) {

		header = recibirHeader(socketNucleo);

		switch (header) {

		case quantumSleep:

			quantumRetardo = recibirCantidadQuantum(socketNucleo);

			break;

		case quantumUnidades:
			quantumTotal = recibirCantidadQuantum(socketNucleo);

			break;

		case headerPcb:

			pcbRecibido = recibirPcb(socketNucleo);

			enviarCambioProcesoActivo(socketUMC, pcbRecibido.pid);
			log_info(logger, "Nuevo proceso activo PID: %d", pcbRecibido.pid);

			int unidadQuantum = 0;
			sigoEjecutando = 1;
			huboSaltoLinea = 0;

			while (unidadQuantum < quantumTotal && sigoEjecutando) {
				t_intructions instruccion = pcbRecibido.indice_codigo.instrucciones[pcbRecibido.pc];
				char lineaAnsisop[instruccion.offset];

				if (pedirLineaAUMC(socketUMC, lineaAnsisop, pcbRecibido, tamanioPagina)) {
					sigoEjecutando = 0;
					log_error(logger, "No se puede seguir ejecutando el programa PID: %d. Fallo en la memoria.", pcbRecibido.pid);
					enviarAbortarProgramaNucleo(socketNucleo);
					enviarPcb(socketNucleo,pcbRecibido);
				} else {
					lineaAnsisop[instruccion.offset] = '\0';

					analizadorLinea(strdup(lineaAnsisop), &functions, &kernel_functions);
					usleep(quantumRetardo * 1000);
					unidadQuantum++;

					if (!huboSaltoLinea) {
						pcbRecibido.pc++;
					} else {
						huboSaltoLinea = 0;
					}
					log_info(logger, "ejecuto linea", texto);

				}

			}
			if (unidadQuantum == quantumTotal && sigoEjecutando) {
				int finQuantum = finDeQuantum;
				send(socketNucleo,&finQuantum,sizeof(int),0);
				enviarPcb(socketNucleo, pcbRecibido);
			}
			break;

		default:
			log_error(logger, "Se produjo error conectando con el Nucleo", texto);
			signalApagado = 1;
		}

	}
	log_info(logger, "Se desconectó la CPU");
	log_destroy(logger);
	config_destroy(config);
	avisarANucleoFinalizacionDeCPU(socketNucleo);
	close(socketNucleo);
	close(socketUMC);
	return 0;

}
//primitivas
//puedo pasar esto a otro archivo

