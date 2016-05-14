/*
 * structs.h
 *
 *  Created on: 14/5/2016
 *      Author: utnso
 */

#ifndef LIBRERIAS_STRUCTS_H_
#define LIBRERIAS_STRUCTS_H_

typedef struct pcb {
	uint32_t pid;
	uint32_t pc;
	uint32_t paginas_codigo;
	uint32_t indice_codigo;
	uint32_t indice_etiquetas;
	uint32_t indice_stack;
} t_pcb;

typedef struct posicion_instruccion {
	uint32_t inicio;
	uint32_t longitud;
};

#endif /* LIBRERIAS_STRUCTS_H_ */
