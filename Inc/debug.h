/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y		 ==
 ==  prototipos de funciones necesarias para el depurado de errores.	 ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */
#ifndef _MSO_DEBUG_H_
#define _MSO_DEBUG_H_

#include <const.h>
#include <tipos.h>

#define FATAL(m)		fatal("FATAL: %s (%d) %s", __FILE__, __LINE__, m)	
/* ====================================================================== *
 == Prototipos de funciones								 ==
 ======================================================================== */
PUBLICA void ver_mem(unt32 direccion, unt32 cantidad);
PUBLICA void ver_regs(regs *r);
PUBLICA void t_debug();
PUBLICA void pausa();
PUBLICA void ver_procs();


/* para provocar errores */
PUBLICA void div0();
PUBLICA void falla_pagina();
PUBLICA void selector_invalido();
PUBLICA void int3();
PUBLICA void fatal(char *s, ...);

#endif
