#include "funciones.h"

int main(int argc, char **argv) {

	t_config * config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
		char * ruta = "./Configuracion/config";
		argv[1] = ruta;
	} else {

		config = config_create(argv[1]);
	}

	int PUERTO_SERVIDOR = config_get_int_value(config, "PUERTO_SERVIDOR");
	int PUERTO_UMC = config_get_int_value(config, "PUERTO_UMC");
	char* IP_UMC = config_get_string_value(config, "IP_UMC");
	uint32_t PAGINAS_STACK = config_get_int_value(config, "PAGINAS_STACK");
	cantidadQuantum = config_get_int_value(config, "QUANTUM");
	retardoQuantum = config_get_int_value(config, "QUANTUM_SLEEP");

	vectorDispositivos = config_get_array_value(config, "IO_ID");
	vectorRetardoDispositivos = config_get_array_value(config, "IO_SLEEP");

	crearHilosEntradaSalida();

	vectorSemaforosAnsisop = config_get_array_value(config, "SEM_ID");
	char ** valoresIniciales = config_get_array_value(config, "SEM_INIT");

	int a = 0;

	while (vectorSemaforosAnsisop[a] != NULL) {
		a++;
	}
	vectorValoresSemaforosAnsisop = malloc(sizeof(uint32_t) * a);
	vectorMutexSemaforosAnsisop = malloc(sizeof(pthread_mutex_t *) * a);
	vectorColasSemaforosAnsisop = malloc(sizeof(t_queue*) * a);

	a = 0;
	while (vectorSemaforosAnsisop[a] != NULL) {
		vectorValoresSemaforosAnsisop[a] = atoi(valoresIniciales[a]);
		vectorMutexSemaforosAnsisop[a] = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(vectorMutexSemaforosAnsisop[a], NULL);
		vectorColasSemaforosAnsisop[a] = queue_create();

		a++;
	}
	vectorVariablesCompartidas = config_get_array_value(config, "SHARED_VARS");
	a = 0;
	while (vectorVariablesCompartidas[a] != NULL) {
		a++;
	}

	vectorValoresVariablesCompartidas = malloc(sizeof(uint32_t) * a);
	vectorMutexVariablesCompartidas = malloc(sizeof(pthread_mutex_t*) * a);
	a = 0;
	while (vectorVariablesCompartidas[a] != NULL) {
		vectorValoresVariablesCompartidas[a] = 0;
		vectorMutexVariablesCompartidas[a] = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(vectorMutexVariablesCompartidas[a], NULL);
		a++;
	}

	sem_init(&semaforoColaListos, 1, 0);

	//Creo log para el Núcleo

	logger = log_create("Nucleo.log", "NUCLEO", 1, log_level_from_string("INFO"));

	texto = "info";

	//Inicializacion notify
	int inotify = inotify_init();
	if (inotify < 0) {
		log_error(logger, "Error inicializando inotify");
	}
	int watch_descriptor = inotify_add_watch(inotify, argv[1], IN_CLOSE_WRITE);

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
	log_info(logger, "Se establecio la conexion con la UMC");

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
	log_info(logger, "Tamaño de pagina recibido: %d bytes", tamanioPagina);

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
	log_info(logger, "Escuchando nuevas conexiones");

	config_destroy(config);

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

	pthread_mutex_init(&mutexUnidadesQuantum, NULL);
	pthread_mutex_init(&mutexRetardoQuantum, NULL);

	pthread_mutex_init(&mutexUMC, NULL);
	pthread_mutex_init(&mutexBolsaSockets, NULL);

	// temp file descriptor list for select()
	int listener = servidorNucleo;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);
	FD_SET(inotify, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	if (listener > inotify) {
		fdmax = listener;
	} else {
		fdmax = inotify;
	}
	int i;

	while (1) {
		pthread_mutex_lock(&mutexBolsaSockets);
		bolsaAuxiliar = bolsaDeSockets; // copy it
		pthread_mutex_unlock(&mutexBolsaSockets);
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
						pthread_mutex_lock(&mutexBolsaSockets);
						FD_SET(nuevaConexion, &bolsaDeSockets);
						pthread_mutex_unlock(&mutexBolsaSockets);
						log_info(logger, "Nueva consola conectada, socket %d", nuevaConexion);
						//Maneja consola

						break;
					case IDCPU:
						;
						pthread_t nuevoHilo;
						pthread_attr_t attr;
						pthread_attr_init(&attr);
						pthread_attr_setdetachstate(&attr,
						PTHREAD_CREATE_DETACHED);
						int * socketConexionParaThread = malloc(sizeof(int));
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

				} else if (i == inotify) {

					char buffer[sizeof(struct inotify_event) + 100];
					read(i, buffer, sizeof(struct inotify_event) + 100);
					struct inotify_event *event = (struct inotify_event *) &buffer[0];
					t_config* cfgAux;
					if (event->mask & IN_CLOSE_WRITE) {
						cfgAux = config_create(argv[1]);
						if (cfgAux != NULL) {
							pthread_mutex_lock(&mutexUnidadesQuantum);
							if (cantidadQuantum != config_get_int_value(cfgAux, "QUANTUM")) {
								cantidadQuantum = config_get_int_value(cfgAux, "QUANTUM");
								log_info("El Quantum se actualizo a: %d\n", (int) cantidadQuantum);
							}
							pthread_mutex_unlock(&mutexUnidadesQuantum);

							pthread_mutex_lock(&mutexRetardoQuantum);
							if (retardoQuantum != config_get_int_value(cfgAux, "QUANTUM_SLEEP")) {
								retardoQuantum = config_get_int_value(cfgAux, "QUANTUM_SLEEP");
								log_info("El Quantum Sleep se actualizo a: %d\n", (int) retardoQuantum);
							}
							pthread_mutex_unlock(&mutexRetardoQuantum);
							config_destroy(cfgAux);
						}
					}
				} else {
					int header = recibirHeader(i);

					switch (header) {

					case programaAnsisop:
						;
						int largoPrograma = recibirLargoProgramaAnsisop(i);
						char *programa = malloc(largoPrograma);
						recibirProgramaAnsisop(i, programa, largoPrograma);

						t_pcb nuevoPcb = crearPcb(programa, largoPrograma);

						pthread_mutex_lock(&mutexUMC);
						int respuestaInicializacion = iniciarUnPrograma(clienteUMC, nuevoPcb, largoPrograma, programa, PAGINAS_STACK);
						pthread_mutex_unlock(&mutexUMC);

						if (respuestaInicializacion == -1) {
							log_error(logger, "Se desconectó UMC. Abortando Núcleo");
							abort();

						} else if (respuestaInicializacion == inicioProgramaError) {

							destruirPcb(nuevoPcb);
							enviarFinalizacionProgramaConsola(i);
							pthread_mutex_lock(&mutexBolsaSockets);
							FD_CLR(i, &bolsaDeSockets);
							pthread_mutex_unlock(&mutexBolsaSockets);
							log_error(logger, "Espacio en memoria insuficiente", texto);

						} else {
							t_pidConConsola *pidConConsola = malloc(sizeof(t_pidConConsola));
							pidConConsola->pid = nuevoPcb.pid;
							pidConConsola->socketConsola = i;
							t_pcbConConsola pcbListo;
							pcbListo.pcb = nuevoPcb;
							pcbListo.socketConsola = i;
							log_info(logger, "Se inicio programa pid %d", nuevoPcb.pid);
							AgregarAProcesoColaListos(pcbListo);
							pthread_mutex_lock(&mutexListaConsolas);
							list_add(listaConsolas, (void *) pidConConsola);
							pthread_mutex_unlock(&mutexListaConsolas);
							free(programa);
						}

						break;

					default:
						pthread_mutex_lock(&mutexBolsaSockets);
						if (!FD_ISSET(i, &bolsaDeSockets)) {
							log_info(logger, "Consola socket %d desconectada", i);
							close(i);
							pthread_mutex_unlock(&mutexBolsaSockets);
							break;
						}
						FD_CLR(i, &bolsaDeSockets);
						pthread_mutex_unlock(&mutexBolsaSockets);
						if (header == finalizacionPrograma) {
							log_info(logger, "Consola socket %d envio finalizacion de programa", i);
						} else {
							log_info(logger, "Consola socket %d desconectada. Finalizando programa", i);
						}
						int j, sizeCola, sizeColaBloqueados, encontrado = 0;
						//Busco pcb en cola de procesos listos
						pthread_mutex_lock(&mutexColaListos);
						sizeCola = queue_size(cola_PCBListos);
						for (j = 0; j < sizeCola; j++) {

							t_pcbConConsola * elementoAux = (t_pcbConConsola *) queue_pop(cola_PCBListos);

							if (elementoAux->socketConsola == i) {
								finalizarProceso(*elementoAux);
								encontrado = 1;
								free(elementoAux);
							} else {
								queue_push(cola_PCBListos, (void *) elementoAux);
							}
						}
						pthread_mutex_unlock(&mutexColaListos);
						int contador = 0, k;
						while (vectorDispositivos[contador] != NULL) {
							contador++;
						}
						//Busco pcb en colas de procesos bloqueados
						for (k = 0; k < contador; k++) {

							pthread_mutex_lock(vectorMutexDispositivosIO[k]);
							sizeColaBloqueados = queue_size(vectorColasBloqueados[k]);

							for (j = 0; j < sizeColaBloqueados; j++) {

								t_pcbBloqueado * elementoAux = (t_pcbBloqueado*) queue_pop(vectorColasBloqueados[k]);

								if (elementoAux->pcb.socketConsola == i) {
									finalizarProceso(elementoAux->pcb);
									encontrado = 1;
									free(elementoAux);
								} else {
									queue_push(vectorColasBloqueados[k], (void *) elementoAux);
								}
							}
							pthread_mutex_unlock(vectorMutexDispositivosIO[k]);
						}

						//Busco en colas de semaforos

						contador = 0;
						while (vectorSemaforosAnsisop[contador] != NULL) {
							contador++;
						}
						//Busco pcb en colas de procesos bloqueados
						for (k = 0; k < contador; k++) {

							pthread_mutex_lock(vectorMutexSemaforosAnsisop[k]);
							sizeColaBloqueados = queue_size(vectorColasSemaforosAnsisop[k]);
							for (j = 0; j < sizeColaBloqueados; j++) {

								t_pcbConConsola * elementoAux = (t_pcbConConsola*) queue_pop(vectorColasSemaforosAnsisop[k]);

								if (elementoAux->socketConsola == i) {
									finalizarProceso(*elementoAux);
									encontrado = 1;
									free(elementoAux);
								} else {
									queue_push(vectorColasSemaforosAnsisop[k], (void *) elementoAux);
								}
							}
							pthread_mutex_unlock(vectorMutexSemaforosAnsisop[k]);
						}

						if (!encontrado) {
							int * socketProcesoFinalizado = malloc(sizeof(int));
							*socketProcesoFinalizado = i;

							pthread_mutex_lock(&mutexListaFinalizacionesPendientes);
							list_add(listaFinalizacionesPendientes, socketProcesoFinalizado);
							pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);
						} else {
							close(i);
						}
						break;
					}
				}
			}
		}
	}
	log_destroy(logger);
	return EXIT_SUCCESS;
}
