/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y		 ==
 ==  prototipos de funciones necesarias para el manejo del buffer cache. ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */
#ifndef _MSO_CACHE_H_
#define _MSO_CACHE_H_

#include <const.h>
#include <tipos.h>

/* ====================================================================== *
 == Constantes										 ==
 ======================================================================== */
#define TAM_BLOQUE	512



/* ====================================================================== *
 == tipos                								 ==
 ======================================================================== */

typedef struct buffer {
  struct buffer * prox;
  struct buffer * ant; 
  struct buffer * hash;
  unt32  bloque;
  int    disp;  
  unt8   estado;
  bool   vital;
  unt8   usos;   
  unt8   d[TAM_BLOQUE];
} buffer;


/* ====================================================================== *
 == prototipos de funciones								 ==
 ======================================================================== */
PUBLICA buffer * obt_buffer(int disp, int bloque, bool vital);
PUBLICA void est_bloque(buffer *b);
PUBLICA void inic_cache();

#endif
