#include "funciones.h"

int main(int argc, char **argv) {

	t_config * config;
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
	uint32_t PAGINAS_STACK = config_get_int_value(config, "PAGINAS_STACK");
	cantidadQuantum = config_get_int_value(config,"QUANTUM");
	retardoQuantum = config_get_int_value(config,"QUANTUM_SLEEP");

	t_queue ** listaColasDispositivos;
	pthread_mutex_t * listaMutexListaDispositivos;
	vectorDispositivos = config_get_array_value(config,"IO_ID");
	vectorRetardoDispositivos = config_get_array_value(config,"IO_SLEEP");

	crearHilosEntradaSalida();


	//Creo log para el Núcleo

	logger = log_create("Núcelo.log", "NUCLEO", 1, log_level_from_string("INFO"));

	texto = "info";

	if (crearSocket(&clienteUMC)) {
		printf("Error creando socket\n");
		log_error(logger, "Se produjo un error creando el socket de UMC", texto);
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
		log_error(logger, "Se produjo un error recibiendo el tamanio de pagina de la UMC", texto);
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
		log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	printf("Escuchando\n");

	pidPcb = 1;

	int nuevaConexion;
	struct sockaddr_in direccionCliente;

	listaConsolas = list_create();
	listaFinalizacionesPendientes = list_create();

	cola_PCBListos = queue_create();
	cola_PCBBloqueados = queue_create();
	cola_PCBFinalizados = queue_create();

	pthread_mutex_init(&mutexColaListos, NULL);
	pthread_mutex_init(&mutexColaFinalizados, NULL);
	pthread_mutex_init(&mutexListaConsolas, NULL);
	pthread_mutex_init(&mutexListaFinalizacionesPendientes, NULL);

	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;  // temp file descriptor list for select()
	int listener = servidorNucleo;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	fdmax = listener;
	int i;

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
					//Espera hasta que el hilo haya guardado el valor que se le paso como parametro
					//antes de sobreEscribir la variable nuevaConexion
					nuevaConexion = aceptarConexion(servidorNucleo, &direccionCliente);

					if (nuevaConexion > fdmax) {
						fdmax = nuevaConexion;
					}

					int idRecibido = iniciarHandshake(nuevaConexion, IDNUCLEO);

					switch (idRecibido) {

					case IDERROR:
						log_info(logger, "Se desconecto el socket", texto);
						close(nuevaConexion);
						break;

					case IDCONSOLA:
						FD_SET(nuevaConexion, &bolsaDeSockets);
						log_info(logger, "Nueva consola conectada", texto);
						//Maneja consola

						break;
					case IDCPU:
						;
						pthread_t nuevoHilo;
						pthread_attr_t attr;
						pthread_attr_init(&attr);
						pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
						int * socketConexionParaThread = malloc(sizeof (int));
						*socketConexionParaThread = nuevaConexion;
						pthread_create(&nuevoHilo, &attr, (void *) manejarCPU, (void *) socketConexionParaThread); //Creo hilo que maneje el nuevo CPU

						pthread_attr_destroy(&attr);

						//list_add(&listaCPUsConectados, (void *) &nuevoHilo);
						log_info(logger, "Nuevo CPU conectado", texto);
						break;
					default:
						close(nuevaConexion);
						log_error(logger, "Error en el handshake. Conexion inesperada", texto);
						break;
					}

				} else {
					int header = recibirHeader(i);

					switch (header) {

					case 0:
						;
						close(i);
						FD_CLR(i, &bolsaDeSockets);
						log_error(logger, "Consola desconectada", texto);
						break;

					case largoProgramaAnsisop:
						;
						int largoPrograma = recibirLargoProgramaAnsisop(i);
						char *programa = malloc(largoPrograma);
						if (recibirHeader(i) == programaAnsisop) {
							recibirProgramaAnsisop(i, programa, largoPrograma);

							t_pcb nuevoPcb = crearPcb(programa, largoPrograma);

							if (iniciarUnPrograma(clienteUMC, nuevoPcb, largoPrograma, programa, PAGINAS_STACK) == inicioProgramaError) {

								printf("No se pudo reservar espacio para el programa");
								free(nuevoPcb.indice_etiquetas.etiquetas);
								free(nuevoPcb.indice_codigo.instrucciones);
								list_destroy(nuevoPcb.indice_stack);

							} else {
								t_pcbConConsola *pcbListo = malloc(sizeof(t_pcbConConsola));
								pcbListo->pcb = nuevoPcb;
								pcbListo->socketConsola = i;
								pcbListo->finalizarPrograma = 0;
								AgregarAProcesoColaListos(*pcbListo);

								pthread_mutex_lock(&mutexListaConsolas);
								list_add(listaConsolas, (void *) pcbListo);
								pthread_mutex_unlock(&mutexListaConsolas);

								free(programa);

							}
						} else {
							close(i);
							FD_CLR(i, &bolsaDeSockets);
							log_error(logger, "La consola no envio un programa", texto);
						}

						break;

					case finalizacionPrograma:
						;
						int j, sizeCola = queue_size(cola_PCBListos), encontrado = 0;

						for (j = 0; j < sizeCola; j++) {

							t_pcbConConsola * elementoAux = (t_pcbConConsola *) queue_pop(cola_PCBListos);

							if (elementoAux->socketConsola == i) {
								finalizarProceso(*elementoAux);
								encontrado = 1;
							} else {
								queue_push(cola_PCBListos, (void *) elementoAux);
							}
						}

						if (!encontrado) {
							int * socketProcesoFinalizado = malloc(sizeof(int));
							*socketProcesoFinalizado = i;

							pthread_mutex_lock(&mutexListaFinalizacionesPendientes);
							list_add(listaFinalizacionesPendientes,socketProcesoFinalizado);
							pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);
						}

						break;

					default:
						close(i);
						FD_CLR(i, &bolsaDeSockets);
						log_error(logger, "Consola desconectada", texto);
						break;

					}
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

//servidor para consola y cpu
//cliente de umc
