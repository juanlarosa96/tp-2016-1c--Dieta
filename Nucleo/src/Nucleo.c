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
	//puts(value);

	//Creo log para el Núcleo
	t_log* logger;
	logger = log_create("Núcelo.log", "NUCLEO", 1,
			log_level_from_string("INFO"));
	char *texto;
	texto = "info";

	/*
	 struct sockaddr_in direccionServidor;
	 direccionServidor.sin_family = AF_INET;
	 direccionServidor.sin_addr.s_addr = INADDR_ANY;
	 direccionServidor.sin_port = htons(PUERTO_SERVIDOR);

	 int servidorNucleo = socket(AF_INET, SOCK_STREAM, 0);
	 */
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

//------------------------------ SELECT

	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;  // temp file descriptor list for select()
	int listener = servidorNucleo;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	fdmax = listener;

	int nuevaConexion;
	struct sockaddr_in direccionCliente;

	char buf[256];    // buffer for client data
	int nbytesRecibidos;
	int i;

	t_list listaCPUsConectados;

	while (1) {
		bolsaAuxiliar = bolsaDeSockets; // copy it
		if (select(fdmax + 1, &bolsaAuxiliar, NULL, NULL, NULL) == -1) {
			perror("select");
			return 1;
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &bolsaAuxiliar)) { // we got one!!
				if (i == listener) {
					// handle new connections
					nuevaConexion = aceptarConexion(i, &direccionCliente);
					int idRecibido = iniciarHandshake(nuevaConexion, IDNUCLEO);

					switch (idRecibido) {
					//case IDERROR: IDERROR no lo reconoce because reasons
					case 0:
						log_info(logger, "Se desconecto el socket", texto);
						close(nuevaConexion);
						break;
					case IDCONSOLA:
						FD_SET(nuevaConexion, &bolsaDeSockets);
						log_info(logger, "Nueva consola conectada", texto);
						break;
					case IDCPU:
						;
						pthread_t nuevoHilo;
						pthread_create(&nuevoHilo, NULL, manejarCPU, (void *) &i); //Creo hilo que maneje el nuevo CPU
						list_add(&listaCPUsConectados, (void *) &nuevoHilo);
						log_info(logger, "Nuevo CPU conectado", texto);
						break;
					default:
						close(nuevaConexion);
						log_error(logger,
								"Error en el handshake. Conexion inesperada",
								texto);
						break;
					}
				}
			} else {
				// Manejo consolas

			}
		}
	}

	return EXIT_SUCCESS;
}

//servidor para consola y cpu
//cliente de umc
