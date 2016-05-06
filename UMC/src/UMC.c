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
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 enum handshake {
 NUCLEO = 3, CPU = 4,
 };
 */

int main(int argc, char *argv[]) {

	//Crear hilo para la consola UMC

	//Recibe el archivo de config por parametro
	if (argc != 2) {
		printf("Número incorrecto de parámetros\n");
		return -1;
	}

	t_config* config = config_create(argv[1]);

	int puerto_servidor = config_get_int_value(config, "PUERTO");
	int puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	char* ip_swap = config_get_string_value(config, "IP_SWAP");

	/*---------SOCKET SERVIDOR------------*/

	struct sockaddr_in direccionServidorUMC;
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

	listen(servidorUMC, 100);

	//clientes de UMC: núcleo, CPU.

	struct sockaddr_in direccionCliente;
	unsigned int len;
	len = sizeof(struct sockaddr_in);

	int socketCliente = accept(servidorUMC, (void*) &direccionCliente, &len);
	printf("Recibí una conexión\n");

	char* buffer = malloc(10);
	int bytesRecibidos = recv(socketCliente, buffer, 10, 0); //Recibe "HEADER: Hola UMC"
	buffer[bytesRecibidos] = '\0';

	/*----------SELECT----------------*/

	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;  // temp file descriptor list for select()
	int listener = servidorUMC;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);
	FD_SET(socketCliente, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	fdmax = (servidorUMC > socketCliente) ? servidorUMC : socketCliente;

	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char buf[256];    // buffer for client data
	int nbytesRecibidos;

	//char remoteIP[INET6_ADDRSTRLEN];

	int i;

	//struct addrinfo hints, *ai, *p;

	for (;;) {
		bolsaAuxiliar = bolsaDeSockets; // copy it
		if (select(fdmax + 1, &bolsaAuxiliar, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &bolsaAuxiliar)) { // we got one!!
				if (i == listener) {
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *) &remoteaddr,
							&addrlen);

					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &bolsaDeSockets); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						/*printf("selectserver: new connection from %s on "
						 "socket %d\n",
						 inet_ntop(remoteaddr.ss_family,
						 get_in_addr((struct sockaddr*)&remoteaddr),
						 remoteIP, INET6_ADDRSTRLEN),
						 newfd);*/
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
						FD_CLR(i, &bolsaDeSockets); // remove from master set
					} else {
						printf("Holis\n");//aca va el tp :D
									}
								}
							}
						}
					}
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
	}

	FALTA IMPLEMENTAR HANDSHAKE
	 char * quienSos = string_substring_until(bufferServidor, 1); //Supongo que el header tiene 1 byte
	 int quienMeHabla = atoi(quienSos); //capaz esto me conviene encapsularlo en una funcion

	 switch(quienMeHabla){
	 case NUCLEO: send(socketCliente, "Hola Núcleo!", 13, 0);
	 break;
	 case CPU: send(socketCliente,"Hola CPU!",10, 0);
	 break;
	 default:
	 send(socketCliente,"No te conozco",100,0);
	 //aca corta la conexion ?
	 }
	 */

	printf("CPU dice: %s\n", buffer);

	/*---------FIN SOCKET SERVIDOR-------*/

	/*---------SOCKET CLIENTE------------*/

	struct sockaddr_in direccionSwap;
	direccionSwap.sin_family = AF_INET;
	direccionSwap.sin_addr.s_addr = inet_addr(ip_swap);
	direccionSwap.sin_port = htons(puerto_swap);

	int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteUMC, (void*) &direccionSwap, sizeof(direccionSwap))
			!= 0) {
		perror("No se pudo conectar con el socket de SWAP");
		return 1;
	}

	//scanf("%s\n", buffer);
	send(clienteUMC, buffer, 10, 0);

	free(buffer);

	/*---------FIN SOCKET CLIENTE-------*/

	config_destroy(config);

	return EXIT_SUCCESS;

}
