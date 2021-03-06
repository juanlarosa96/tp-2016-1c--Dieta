/*
 * hexdump.c
 *
 *  Created on: 9/6/2016
 *      Author: utnso
 */

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 16
#endif

#include "hexdump.h"

void hexdump(FILE* archivo, void *memoria, unsigned int len)
{
        unsigned int i, j;

        for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
        {
                /* print offset */
                if(i % HEXDUMP_COLS == 0)
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
                if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1))
                {
                        for(j = i - (HEXDUMP_COLS - 1); j <= i; j++)
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
