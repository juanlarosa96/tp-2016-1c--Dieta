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

//#include <sockets.h>

int main(int argc, char *argv[]) {

	t_config* config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
	} else {

		config = config_create(argv[1]);
	}

	int frames = config_get_int_value(config,"MARCOS");
	int cant_frames = config_get_int_value(config, "MARCOS_SIZE");
	int memoriaDisponible = frames * cant_frames;

	int * memoria = malloc(memoriaDisponible);
	memset(memoria,0,sizeof(memoriaDisponible)); //en el tercer parametro va memoria o memoria disponible?

	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char* ip_swap = config_get_string_value(config, "IP_SWAP");


	/*---------SOCKET SERVIDOR------------*/

	//servidor de Nucleo y CPUs
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

	/*---------SOCKET CLIENTE------------*/
	//cliente de Swap

	int clienteSwap;
	if (crearSocket(&clienteSwap)) {
		printf("Error creando socket\n");
		//log_error(logger, "Se produjo un error creando el socket de UMC", texto);
		return 1;
	}
	if (conectarA(clienteSwap, ip_swap, puerto_swap)) {
		printf("Error al conectar\n");
		//log_error(logger, "Se produjo un error conectandose a la UMC", texto);
		return 1;
	}

	/*struct sockaddr_in direccionSwap;
	 direccionSwap.sin_family = AF_INET;
	 direccionSwap.sin_addr.s_addr = inet_addr(ip_swap);
	 direccionSwap.sin_port = htons(puerto_swap);

	 int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	 if (connect(clienteUMC, (void*) &direccionSwap, sizeof(direccionSwap))
	 != 0) {
	 perror("No se pudo conectar con el socket de SWAP");
	 return 1;
	 }

	 char* buffer = malloc(10);
	 scanf("%s\n", buffer);
	 send(clienteUMC, buffer, 10, 0);

	 free(buffer);*/

	/*struct sockaddr_in direccionServidorUMC;
	 direccionServidorUMC.sin_family = AF_INET;
	 direccionServidorUMC.sin_addr.s_addr = INADDR_ANY;
	 direccionServidorUMC.sin_port = htons(puerto_servidor);

	 int servidorUMC = socket(AF_INET, SOCK_STREAM, 0);

	 int activado = 1;
	 setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado,
	 sizeof(activado));

	 if (bind(servidorUMC, (void*) &direccionServidorUMC,
	 sizeof(direccionServidorUMC)) != 0) {
	 perror("Falló el bind");
	 return 1;
	 }

	 printf("Estoy escuchando\n");

	 listen(servidorUMC, 100);*/

	//clientes de UMC: núcleo, CPU.
	/*struct sockaddr_in direccionCliente;
	 unsigned int len;
	 len = sizeof(struct sockaddr_in);

	 int socketCliente = accept(servidorUMC, (void*) &direccionCliente, &len);
	 printf("Recibí una conexión\n");
	 */

	//int bytesRecibidos = recv(socketCliente, buffer, 10, 0); //Recibe "HEADER: Hola UMC"
	//buffer[bytesRecibidos] = '\0';
	/*----------SELECT----------------*/

	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;  // temp file descriptor list for select()
	int listener = servidorUMC;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);
	//FD_SET(socketCliente, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	//fdmax = (servidorUMC > socketCliente) ? servidorUMC : socketCliente;

	fdmax = listener;

	int nuevaConexion;        // newly accept()ed socket descriptor
	struct sockaddr_in direccionCliente;

	//struct sockaddr_storage remoteaddr; // client address
	//socklen_t addrlen;

	char buf[256];    // buffer for client data
	int nbytesRecibidos;

	//char remoteIP[INET6_ADDRSTRLEN];

	int i;

	//struct addrinfo hints, *ai, *p;

	while (1) {
		bolsaAuxiliar = bolsaDeSockets; // copy it
		if (select(fdmax + 1, &bolsaAuxiliar, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &bolsaAuxiliar)) { // we got one!!
				if (i == listener) {
					nuevaConexion = aceptarConexion(i, &direccionCliente);
					int idRecibido = iniciarHandshake(nuevaConexion, IDUMC);

					switch (idRecibido) {
					case 0:
						//log_info(logger, "Se desconecto el socket", texto);
						close(nuevaConexion);
						break;
					case IDCPU:
						FD_SET(nuevaConexion, &bolsaDeSockets);
						pthread_t nuevoHilo;
						//pthread_create(&nuevoHilo, NULL,(void *) &manejarCPU, (void *) &i);
						//Creo hilo que maneje el nuevo CPU
						//log_info(logger, "Nuevo CPU conectado", texto);
						break;
					default:
						close(nuevaConexion);
						//log_error(logger, "Error en el handshake. Conexion inesperada", texto);
						break;
					}
				} else {
					printf("Holis\n");						//aca va el tp :D
				}
			}
		}
	}


	// handle new connections
	/*addrlen = sizeof remoteaddr;
	 newfd = accept(listener, (struct sockaddr *) &remoteaddr,
	 &addrlen);


	 if (newfd == -1) {
	 perror("accept");


	 } else {
	 FD_SET(newfd, &bolsaDeSockets); // add to master set
	 if (newfd > fdmax) {    // keep track of the max
	 fdmax = newfd;
	 }
	 printf("Nueva Conexion\n");
	 printf("selectserver: new connection from %s on "
	 "socket %d\n",
	 inet_ntop(remoteaddr.ss_family,
	 get_in_addr((struct sockaddr*)&remoteaddr),
	 remoteIP, INET6_ADDRSTRLEN),
	 newfd);
	 }
	 } else {
	 // handle data from a client
	 if ((nbytesRecibidos = recv(i, buf, sizeof buf, 0)) <= 0) {
	 // got error or connection closed by client
	 if (nbytesRecibidos == 0) {
	 // connection closed
	 printf("selectserver: socket %d hung up\n", i);
	 } else {
	 perror("recv");
	 }
	 close(i); // bye!
	 FD_CLR(i, &bolsaDeSockets); // remove from master set*/

	// END handle data from client
	// END got new incoming connection
	// END looping through file descriptors
	// END for(;;)--and you thought it would never end!
	/*memset(&hints, 0, sizeof hints);
	 hints.ai_family = AF_UNSPEC;
	 hints.ai_socktype = SOCK_STREAM;
	 hints.ai_flags = AI_PASSIVE;

	 if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
	 fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
	 exit(1);
	 }

	 for (p = ai; p != NULL; p = p->ai_next) {
	 listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	 if (listener < 0) {
	 continue;
	 }

	 // lose the pesky "address already in use" error message
	 setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	 if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
	 close(listener);
	 continue;
	 }

	 break;
	 }*/

	//printf("CPU dice: %s\n", buffer);


	config_destroy(config);

	return EXIT_SUCCESS;

}
