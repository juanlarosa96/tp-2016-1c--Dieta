/*
 * hexdump.c
 *
 *  Created on: 9/6/2016
 *      Author: utnso
 */


#include "hexdump.h"

void hexdump(FILE* archivo, void *memoria, unsigned int len, unsigned int columnas)
{
        unsigned int i, j;

        for(i = 0; i < len + ((len % columnas) ? (columnas - len % columnas) : 0); i++)
        {
                /* print offset */
                if(i % columnas == 0)
                {
                        fprintf(archivo,"0x%06x: ", i);
                }

                /* print hex data */
                if(i < len)
                {
                        fprintf(archivo, "%02x ", 0xFF & ((char*)memoria)[i]);
                }
                else /* end of block, just aligning for ASCII dump */
                {
                        fprintf(archivo, "   ");
                }

                /* print ASCII dump */
                if(i % columnas == (columnas - 1))
                {
                        for(j = i - (columnas - 1); j <= i; j++)
                        {
                                if(j >= len) /* end of block, not really printing */
                                {
                                        fputc(' ', archivo);
                                }
                                else if(isprint(((char*)memoria)[j])) /* printable char */
                                {
                                        fputc(0xFF & ((char*)memoria)[j],archivo);
                                }
                                else /* other char */
                                {
                                        fputc('.',archivo);
                                }
                        }
                        fputc('\n', archivo);
                }
        }
}
