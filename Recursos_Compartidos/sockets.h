
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>

    int crearSocket(int *unSocket);
    int escucharEn(int *unSocket, int puerto);
    int conectarA(int *unSocket, char* ip, int puerto);



