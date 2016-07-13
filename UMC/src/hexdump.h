/*
 * hexdump.h
 *
 *  Created on: 9/6/2016
 *      Author: utnso
 */

#ifndef HEXDUMP_H_
#define HEXDUMP_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void hexdump(FILE * archivo, void *memoria, unsigned int len, unsigned int columnas);

#endif /* HEXDUMP_H_ */
