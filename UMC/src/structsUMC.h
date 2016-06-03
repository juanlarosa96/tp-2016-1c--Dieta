/*
 * structs.h
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <commons/collections/list.h>

typedef struct {
	uint32_t pid;
	uint32_t nroPagina;
	uint32_t nroFrame;
	uint16_t ultAcceso;
}t_entrada_tlb;

struct nodo_lista_frames {
	uint32_t nroFrame;
	uint32_t pid;
	//uint32_t nroPagina;
	uint8_t bitReferencia;
	uint8_t bitModificado;

};

typedef struct nodo_lista_frames t_nodo_lista_frames;

struct nodo_lista_paginas {
	uint32_t nro_pagina;
	char status;
	uint32_t nroFrame;// M = memoria, S = SWAP
};

typedef struct nodo_lista_paginas t_nodo_lista_paginas;

struct nodo_lista_procesos {
	uint32_t pid;
	uint32_t cantPaginas;
	uint8_t framesAsignados;
	int punteroClock;
	t_list * lista_paginas;
};

typedef struct nodo_lista_procesos t_nodo_lista_procesos;


#endif /* STRUCTS_H_ */
