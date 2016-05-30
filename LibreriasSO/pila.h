/*
 * pila.h
 *
 *  Created on: 28/5/2016
 *      Author: utnso
 */

#ifndef PILA_H_
#define PILA_H_

#include "structs.h"

t_registro_pila *popPila(t_list *pila);
void pushPila(t_list * pila, t_registro_pila *elementoPila);

#endif /* PILA_H_ */
