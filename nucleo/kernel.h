/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias el nucleo.                       ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#ifndef _MSO_KERNEL_H_
#define _MSO_KERNEL_H_

#include <const.h>
#include <tipos.h>

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define VER_MAYOR       0
#define VER_MENOR       0
#define VER_SUB         'a'
#define ANNO            2003
                        
#define BASE_NUCLEO     0x10000


/* ====================================================================== *
 == Estructuras                                                          ==
 ======================================================================== */
EXTERNA void girar();      /* en C0.S hace girar la rueda */

#endif
