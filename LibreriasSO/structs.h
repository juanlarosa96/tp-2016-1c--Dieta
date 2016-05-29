/*
 * structs.h
 *
 *  Created on: 14/5/2016
 *      Author: utnso
 */

#ifndef LIBRERIAS_STRUCTS_H_
#define LIBRERIAS_STRUCTS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/metadata_program.h>
#include <stdint.h>

typedef struct {
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_posicion_memoria;


typedef struct {
	t_list lista_argumentos;
	t_list lista_variables;
	uint32_t direccion_retorno;
	t_posicion_memoria variable_retorno;
} t_registro_pila;

/*typedef struct t_pila{
	t_registro_pila registro;
	struct t_pila * indice_stack;
	int indicePila;
} t_pila;*/

typedef struct {
	t_size largoTotalEtiquetas;
	char* etiquetas;
}t_indiceEtiquetas;

typedef struct {
	t_intructions * instrucciones;
	t_size cantidadInstrucciones;
	t_puntero_instruccion numeroInstruccionInicio;
}t_indiceCodigo;

typedef struct {
	uint32_t pid;
	uint32_t pc;
	uint32_t paginas_codigo;
	t_indiceCodigo indice_codigo;
	t_indiceEtiquetas indice_etiquetas;
	t_list * indice_stack;
} t_pcb;



#endif /* LIBRERIAS_STRUCTS_H_ */
