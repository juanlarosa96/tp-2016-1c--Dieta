#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funciones.h"

int main(int argc, char **argv) {

	t_config* config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}

	int PUERTO_SERVIDOR = config_get_int_value(config, "PUERTO_SERVIDOR");
	int PUERTO_UMC = config_get_int_value(config, "PUERTO_UMC");
	char* IP_UMC = config_get_string_value(config, "IP_UMC");


	//Creo log para el Núcleo
	t_log* logger;
	logger = log_create("Núcelo.log", "NUCLEO", 1,
			log_level_from_string("INFO"));
	char *texto;
	texto = "info";


	int clienteUMC;
	if (crearSocket(&clienteUMC)) {
		printf("Error creando socket\n");
		log_error(logger, "Se produjo un error creando el socket de UMC",
				texto);
		return 1;
	}
	if (conectarA(clienteUMC, IP_UMC, PUERTO_UMC)) {
		printf("Error al conectar\n");
		log_error(logger, "Se produjo un error conectandose a la UMC", texto);
		return 1;
	}

	if (responderHandshake(clienteUMC, IDNUCLEO, IDUMC)) {
		log_error(logger, "Error en el handshake", texto);
		return 1;

	}
	if (recibirHeader(clienteUMC) == tamanioDePagina) {
		tamanioPagina = recibirTamanioPagina(clienteUMC);
	} else {
		printf("Error recibiendo tamanio pagina");
		log_error(logger,
				"Se produjo un error recibiendo el tamanio de pagina de la UMC",
				texto);
		return 1;

	}

	log_info(logger, "Se estableció la conexion con la UMC", texto);

	int servidorNucleo;
	if (crearSocket(&servidorNucleo)) {
		printf("Error creando socket");
		return 1;
	}
	if (escucharEn(servidorNucleo, PUERTO_SERVIDOR)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error creando el socket servidor",
				texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	printf("Escuchando\n");

	pidPcb = 1;

	int nuevaConexion;
	struct sockaddr_in direccionCliente;



	t_list listaCPUsConectados;
	t_list * aux = list_create();
	listaConsolas = *aux;
	free(aux);

	cola_PCBListos = queue_create();
	cola_PCBBloqueados= queue_create();
	cola_PCBFinalizados= queue_create();

	while (1) {

		// Fuera del select
		nuevaConexion = aceptarConexion(servidorNucleo, &direccionCliente);
		int idRecibido = iniciarHandshake(nuevaConexion, IDNUCLEO);

		switch (idRecibido) {
		//case IDERROR: IDERROR no lo reconoce because reasons
		case 0:
			log_info(logger, "Se desconecto el socket", texto);
			close(nuevaConexion);
			break;

		case IDCONSOLA:
			log_info(logger, "Nueva consola conectada", texto);

			int header = recibirHeader(nuevaConexion);
			if (header == largoProgramaAnsisop) {
				int largoPrograma = recibirLargoProgramaAnsisop(nuevaConexion);
				char *programa;
				recibirProgramaAnsisop(nuevaConexion, programa, largoPrograma);

				t_pcb nuevoPcb = crearPcb(programa, largoPrograma);

				if (iniciarUnPrograma(clienteUMC, nuevoPcb, largoPrograma,
						programa) == inicioProgramaError) {

					printf("No se pudo reservar espacio para el programa");

				} else {
					t_pcbConConsola pcbListo;
					pcbListo.pcb = nuevoPcb;
					pcbListo.socketConsola = nuevaConexion;
					AgregarAProcesoColaListos(pcbListo);
					//Sincronizar
					list_add(&listaConsolas,(void *) &pcbListo);
					//---
				}

			}

			if (header <= 0) {
				//Cerrar conexion

			}

			break;
		case IDCPU:
			;
			pthread_t nuevoHilo;
			//Hacer thread desechable
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

			pthread_create(&nuevoHilo, &attr, (void *) &manejarCPU, (void *) &nuevaConexion); //Creo hilo que maneje el nuevo CPU

			pthread_attr_destroy(&attr);


			list_add(&listaCPUsConectados, (void *) &nuevoHilo);
			log_info(logger, "Nuevo CPU conectado", texto);
			break;
		default:
			close(nuevaConexion);
			log_error(logger, "Error en el handshake. Conexion inesperada",
					texto);
			break;
		}

	}

	return EXIT_SUCCESS;
}

//servidor para consola y cpu
//cliente de umc
