/*
 * pila.c
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */

#include "pila.h"


t_registro_pila * popPila(t_list *pila){

	void * elementoPop = list_remove(pila,list_size(pila)-1);
	t_registro_pila * elementoPila;

	if(elementoPop == NULL){
		return (t_registro_pila *)elementoPop;
	}
	elementoPila = malloc(sizeof(t_registro_pila));
	memcpy(elementoPila,elementoPop,sizeof(t_registro_pila));
	return elementoPila;
}

void pushPila(t_list * pila, t_registro_pila *elementoPila){

	list_add(pila,(void *) elementoPila);
}
