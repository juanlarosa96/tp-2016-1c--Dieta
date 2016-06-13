/*
 * pila.c
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */

#include "pila.h"

t_registro_pila * popPila(t_list *pila) {

	if (list_size(pila) == 0) {
		return NULL;
	} else {
		void * elementoPop = list_remove(pila, list_size(pila) - 1);
		return (t_registro_pila *) elementoPop;
	}
}

void pushPila(t_list * pila, t_registro_pila *elementoPila) {

	list_add(pila, (void *) elementoPila);
}
