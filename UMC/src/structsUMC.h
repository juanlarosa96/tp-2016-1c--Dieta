/*
 * structs.h
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <commons/collections/list.h>

struct nodo_lista_frames {
	uint32_t pid;
	uint8_t bitReferencia;
	uint8_t bitModificado;
	//uint8_t paginaModificada;
};

typedef struct nodo_lista_frames t_nodo_lista_frames;

struct nodo_lista_paginas {
	uint8_t nro_pagina;
	char status; // M = memoria, S = SWAP
};

typedef struct nodo_lista_paginas t_nodo_lista_paginas;

struct nodo_lista_procesos {
	uint32_t pid;
	uint8_t cantPaginas;
	//uint8_t paginasAsignadas;
	t_list * lista_paginas;
};

typedef struct nodo_lista_procesos t_nodo_lista_procesos;


#endif /* STRUCTS_H_ */
