/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo de las excs.      ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================= */

#ifndef _MSO_EXCEP_H_
#define _MSO_EXCEP_H_

#include <tipos.h>


/* ====================================================================== *
 == Tipos                                                                ==
 ======================================================================= */
typedef void (*fnc_man)(regs *);

typedef struct {
   unt8     nro;
   fnc_man  fnc;
   char    *descripcion;
   char    *ayuda;
}  exc_s;


/* ====================================================================== *
 == Prototipos de funciones                                              ==
 ======================================================================= */
PUBLICA void inic_excs();


/*
 * Funciones para las excepciones. 
 * Definidas en c0.asm
 */
EXTERNA void division_cero(); 
EXTERNA void paso_a_paso(); 
EXTERNA void inem(); 
EXTERNA void break_point(); 
EXTERNA void sobreflujo(); 
EXTERNA void limite(); 
EXTERNA void op_no_valida(); 
EXTERNA void cp_no_disp(); 
EXTERNA void doble_falla(); 
EXTERNA void cp_seg(); 
EXTERNA void tss_ausente(); 
EXTERNA void seg_ausente(); 
EXTERNA void exc_pila(); 
EXTERNA void prot_gral(); 
EXTERNA void fallo_pagina(); 
EXTERNA void cp_error(); 
EXTERNA void int_00();
EXTERNA void int_01();
EXTERNA void int_02();
EXTERNA void int_03();
EXTERNA void int_04();
EXTERNA void int_05();
EXTERNA void int_06();
EXTERNA void int_07();
EXTERNA void int_08();
EXTERNA void int_09();
EXTERNA void int_10();
EXTERNA void int_11();
EXTERNA void int_12();
EXTERNA void int_13();
EXTERNA void int_14();
EXTERNA void int_15();
EXTERNA void int_sistema();
#endif
