/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo de la consola.    ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#ifndef _MSO_TERMINAL_H_
#define _MSO_TERMINAL_H_

#include <const.h>
#include <tipos.h>
#include "..\proc.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define TAM_BUFFER_ENT     64
#define TAM_BUFFER_SAL  (512)


/* ====================================================================== *
 == Estructuras                                                          ==
 ======================================================================== */
typedef struct {
   int   nro_entradas;
   unt8  buf_entrada[TAM_BUFFER_ENT]; /* buffer para las teclas precionadas */
   unt8 *p_cabeza;
   unt8 *p_cola;
   unt8  estado;


   unt8  buf_salida[TAM_BUFFER_SAL];  /* buffer para guardar las salidas    */ 
   int   nro_aperturas;

   bool  aviso;
   bool  escape;

   proc *llamador;
   dir32 buffer;
   int   cnt_leer;
   int   cnt_escribir;

   void *disp;
} terminal;



/* ====================================================================== *
 == Prototipo de funciones                                               ==
 ======================================================================== */
PUBLICA terminal * term_actual;
PUBLICA void manejar_avisos(terminal * t);
PUBLICA void term_procesar(terminal *t, char *b, int c);
PUBLICA void inic_terms();

#endif
