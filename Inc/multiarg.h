/***********************************************************************************************
*   multiarg.h
************************************************************************************************
*   Se utiliza para manejar multiples argumentos.
*   Algunas de las funciones que la utilizan son:
*      - formatear
*   
*   problemas:
*      - no esta testeado.
*      - no detecta el ultimo argumento por lo que es posible leer la pila indefinidamente.
**********************************************************************************************/

#ifndef _MSO_MULTIARG_H_
#define _MSO_MULTIARG_H_

#include "tipos.h"

#define l_arg   void *
#define i_arg(ult)   \
        (unt8 *)&ult + sizeof(ult)

#define arg(ar, tipo)   \
        *((tipo *)((tipo *)ar++))

#define f_arg(arg)

#endif

