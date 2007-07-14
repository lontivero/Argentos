/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo de la consola.    ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#ifndef _MSO_CONSOLA_H_
#define _MSO_CONSOLA_H_

#include <const.h>
#include <debug.h>
#include "terminal.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
/* para en manejo de la memoria de video */
#define NRO_FILAS_VIDEO    25
#define NRO_COLUMNAS_VIDEO 80

#define DIM_VIDEO        (NRO_FILAS_VIDEO * NRO_COLUMNAS_VIDEO)
#define BYTES_FILA       (2 * NRO_COLUMNAS_VIDEO)
#define BYTES_VIDEO      (2 * DIM_VIDEO)


#define NRO_CONSOLAS     (TAM_VIDEO / DIM_VIDEO) / 4
#define TAM_PAGINA_VIDEO (TAM_VIDEO / 1)

#define TAM_BUFFER_VIDEO NRO_COLUMNAS_VIDEO   /* una fila    */

#define TAB              0x08
#define TAB_TAM             8
#define TAB_MSK             7

/* para en manejo de los colores */
#define COL_NEGRO        0x00
#define COL_AZUL         0x01
#define COL_VERDE        0x02
#define COL_CYAN         0x03
#define COL_ROJO         0x04
#define COL_MAGENTA      0x05
#define COL_AMARILLO     0x06
#define COL_BLANCO       0x07


#define COLOR(fondo, texto)    ((char) ((fondo) << 4) | (texto))


/* ====================================================================== *
 == Tipos                                                                ==
 ======================================================================== */
typedef struct {
   /* para el control de la memoria de video                               */
   /* NOTA: estos se calculan relativos a la mem. de video y en palabras   */
   unt16 tope_sup;      /* off del tope superior del bloq. de mem. de video*/
                        /* asignado a esta consola.                        */
   unt16 tope_inf;      /* off. del tope inferior. IDEM tope_sup           */
   unt16 origen;        /* off del origen de la consola.                   */
 
   /* para la posicion */
   int   posx, posy;     /* columna y fila respectivamente                 */
   unt16 pos_cur;        /* posicion del cursor                            */
 
   unt16 atrib;          /* atributo actual de los caracteres              */
   terminal * term;
 
   unt16 cnt_ram;        /* cantidad de caracteres en el buffer            */
   unt16 mem_ram[TAM_BUFFER_VIDEO]; /* buffer                              */
}  cons;


/* ====================================================================== *
 == Prototipo de funciones                                               ==
 ======================================================================== */
PUBLICA void limpiar();
PUBLICA void color(char c);
PUBLICA void mostrar(char *s, ...);
PUBLICA void cambiar_consola(int c);
PUBLICA void escribir_consola(terminal *t);
PUBLICA void inic_consola(terminal * t);

#endif
