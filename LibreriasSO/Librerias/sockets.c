#include "sockets.h"

int escucharEn(int unSocket, int puerto) {

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(puerto);
	//memset(&(address.sin_zero), '\0', 8); Veriguar si falta '\0'

	if ((bind(unSocket, (struct sockaddr *) &address, sizeof(struct sockaddr)))
			== 1) {
		//No se puedo ejecutar bind en el socket
		return 1;
	}

	if ((listen(unSocket, 100)) == 1) {
		//No se puedo ejecutar listen en el socket
		return 1;
	}

	return 0;
}

int conectarA(int unSocket, char * ip, int puerto) {

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(puerto);
	//memset(&(address.sin_zero), '\0', 8); Ver si le falta el '\0'

	if (connect(unSocket, (struct sockaddr *) &address, sizeof(struct sockaddr))
			!= 0) {
		//No se puedo conectar el socket
		return 1;
	}

	return 0;
}

int crearSocket(int *unSocket) {

	if ((*unSocket = socket(AF_INET, SOCK_STREAM, 0)) == 1) {
		//No se puedo crear el socket
		return 1;
	} else {

		int yes = 1;
		if ((setsockopt(*unSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
				== 1) {
			//Error seteando el socket
			return 1;
		}

		return 0;
	}
}

