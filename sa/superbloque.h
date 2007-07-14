/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y		 ==
 ==  prototipos de funciones necesarias para el manejo del super bloque. ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */
#ifndef _MSO_CACHE_H_
#define _MSO_CACHE_H_

#include "const.h"
#include "tipos.h"

/* ====================================================================== *
 == Constantes										 ==
 ======================================================================== */


/* ====================================================================== *
 == tipos                								 ==
 ======================================================================== */
/*
   La cant. de bloques usados para el mapa de bits esta en relacion con el
   tamano del dispositivo.  Asi, un disquette de  1,44 MB (2880 bloques de
   512 bytes) tendra 2880 bits (360 bytes) en su mapa. Esto es 1 bloque.
   
   cnt. blqs. = (tam. disp.)/(tam. bloque)
   cnt. blqs. mapa de bits = (cnt. blqs./8)/(tam. blq)

   cnt. blqs. nodos i = ((cant. nodos i)*(tam. nodos i))/(tam. blq)

   primer blq. de datos = 2 + (cnt. blqs. mapa de bits) + (cnt. blqs. nodos i)

   bloques		pos	tam (en blqs)
   -------        ---	---
   b. booteo	0	1
   b. super		1	1
   b. mapa bits	2	x1
   b. nodos i	x1	x2
   b. de datos	x2	xf

   los nodos i
   -----------
   i(0) prg de arranque.
   i(1) blqs defectuosos.
   i(2) mapa de bits.
   i(3) dir. raiz.

   los atributos
   -------------
   del sistema		00000001	1
   solo lectura		00000010	2
   dispositivo		00000100	4
   directorio		00001000	8
   de archivo		00010000	10
   ejecutable		00100000	20
*/

typedef struct {
  char 	magico[3];		/* nro. magico 'SB\0'	*/
  unt32	cnt_nodos_i;
  unt32	tam;

} superbloque;


/* ====================================================================== *
 == prototipos de funciones								 ==
 ======================================================================== */

#endif
