/***********************************************************************************************
*   mem.h
************************************************************************************************
*   Declaracion de funciones para la manipulacion de memoria
*
**********************************************************************************************/

#ifndef _MSO_MEM_H_
#define _MSO_MEM_H_

#include "tipos.h"

void cp_mem(dir32 destino, dir32 origen, unt32 cantidad ); 
void cp_memx(dir32 destino, dir32 origen, unt32 cantidad );
void pt_mem(dir32 destino, int8 c, unt32 cantidad);

unt32 tam_cad(char * cad);
void  cp_cad(char * destino, char * origen);

#endif
