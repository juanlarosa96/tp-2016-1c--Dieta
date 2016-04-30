#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
	//Recibe el archivo de config por parametro
	if (argc != 2) {
		printf("Número incorrecto de parámetros\n");
		return -1;
	}

	t_config* config = config_create(argv[1]);

	int puerto_umc = config_get_int_value(config,"PUERTO_UMC");
	int puerto_nucleo = config_get_int_value(config,"PUERTO_NUCLEO");
	char* ip_umc = config_get_string_value(config,"IP_UMC");
	char* ip_nucleo = config_get_string_value(config,"IP_NUCLEO");

	//socket cliente nucleo
	struct sockaddr_in direccionServidorNucleo;
	direccionServidorNucleo.sin_family = AF_INET;
	direccionServidorNucleo.sin_addr.s_addr = inet_addr(ip_nucleo);
	direccionServidorNucleo.sin_port = htons(puerto_nucleo);

	int clienteNucleo = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteNucleo, (void*) &direccionServidorNucleo, sizeof(direccionServidorNucleo)) != 0) {
		perror("No se pudo conectar al nucleo");
		return 1;
	}

	/*char mensaje[1000];
	scanf("%s", mensaje);*/

	char* buffer = malloc(100);
	int bytesRecibidos = recv(clienteNucleo, buffer, 100, 0);
	buffer[bytesRecibidos] = '\0';
	printf("%s\n", buffer);


	//cliente de UMC
	struct sockaddr_in direccionServidorUMC;
	direccionServidorUMC.sin_family = AF_INET;
	direccionServidorUMC.sin_addr.s_addr = inet_addr(ip_umc);
	direccionServidorUMC.sin_port = htons(puerto_umc);

	int clienteUMC = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
		perror("No se pudo conectar a la UMC");
		return 1;
	}


	send(clienteUMC, buffer, strlen(buffer), 0);

	/*char mensajeUMC[1000];
	scanf("%s", mensajeUMC);

	send(clienteUMC, mensajeUMC, strlen(mensajeUMC), 0);*/

	return 0;

}
