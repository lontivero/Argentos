/* ====================================================================== *
 ==  Este archivo contiene las funciones nacesarias para manejar las     ==
 ==  excepciones.                                                        ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#include <macros.h>
#include "mprot.h"
#include "excep.h"
#include "irq.h"
#include "excep.ayu"
#include "disp\consola.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define DIVISION_CERO         0
#define PASO_A_PASO           1
#define INEM                  2
#define BREAK_POINT           3
#define SOBREFLUJO            4
#define LIMITE                5
#define OP_NO_VALIDA          6
#define CP_NO_DISP            7
#define DOBLE_FALLA           8
#define CP_SEG                9
#define TSS_AUSENTE          10
#define SEGM_AUSENTE         11
#define EXC_PILA             12
#define PROT_GRAL            13
#define FALLO_PAGINA         14
#define CP_ERROR             15

/* Privilegios. */
#define INTR_PRIVILEGE   (PRESENTE | TIPO_INTR_GATE | PRIV_KERNEL)
#define USER_PRIVILEGE   (PRESENTE | TIPO_INTR_GATE | PRIV_USUARIO)



/* ====================================================================== *
 == Tipos                                                                ==
 ======================================================================== */
typedef struct {
   void  (*fnc)(void); /* funcion a invocar */
   unt8  vec_nr;       /* nro de interrupcion a asignar */
   unt16 privilegios;  /* privilegios para esta int. */
}  gate_table_s;


/* ====================================================================== *
 == Esta es la tabla de ints y excs.                                     ==
 == Las funciones division_cero, paso_a_paso, ..... estan en c0.asm y    ==
 == declaradas externas en excep.h.                                      ==
 ======================================================================== */
gate_table_s gate_table[32] = {
   { division_cero, DIVISION_CERO,  INTR_PRIVILEGE },
   { paso_a_paso,   PASO_A_PASO,    INTR_PRIVILEGE },
   { inem,          INEM,           INTR_PRIVILEGE },
   { break_point,   BREAK_POINT,    USER_PRIVILEGE },
   { sobreflujo,    SOBREFLUJO,     USER_PRIVILEGE },
   { limite,        LIMITE,         INTR_PRIVILEGE },
   { op_no_valida,  OP_NO_VALIDA,   INTR_PRIVILEGE },
   { cp_no_disp,    CP_NO_DISP,     INTR_PRIVILEGE },
   { doble_falla,   DOBLE_FALLA,    INTR_PRIVILEGE },
   { cp_seg,        CP_SEG,         INTR_PRIVILEGE },
   { tss_ausente,   TSS_AUSENTE,    INTR_PRIVILEGE },
   { seg_ausente,   SEGM_AUSENTE,   INTR_PRIVILEGE },
   { exc_pila,      EXC_PILA,       INTR_PRIVILEGE },
   { prot_gral,     PROT_GRAL,      INTR_PRIVILEGE },
   { fallo_pagina,  FALLO_PAGINA,   INTR_PRIVILEGE },
   { cp_error,      CP_ERROR,       INTR_PRIVILEGE },
   { int_00,        IRQ_VEC( 0),    INTR_PRIVILEGE },
   { int_01,        IRQ_VEC( 1),    INTR_PRIVILEGE },
   { int_02,        IRQ_VEC( 2),    INTR_PRIVILEGE },
   { int_03,        IRQ_VEC( 3),    INTR_PRIVILEGE },
   { int_04,        IRQ_VEC( 4),    INTR_PRIVILEGE },
   { int_05,        IRQ_VEC( 5),    INTR_PRIVILEGE },
   { int_06,        IRQ_VEC( 6),    INTR_PRIVILEGE },
   { int_07,        IRQ_VEC( 7),    INTR_PRIVILEGE },
   { int_08,        IRQ_VEC( 8),    INTR_PRIVILEGE },
   { int_09,        IRQ_VEC( 9),    INTR_PRIVILEGE },
   { int_10,        IRQ_VEC(10),    INTR_PRIVILEGE },
   { int_11,        IRQ_VEC(11),    INTR_PRIVILEGE },
   { int_12,        IRQ_VEC(12),    INTR_PRIVILEGE },
   { int_13,        IRQ_VEC(13),    INTR_PRIVILEGE },
   { int_14,        IRQ_VEC(14),    INTR_PRIVILEGE },
   { int_15,        IRQ_VEC(15),    INTR_PRIVILEGE }
}; 




/* ====================================================================== *
 == Todas las excepciones llaman a manejador. Este simplemente muestra   ==
 == el contenido de los registros cuando se produjo la excepcion. Asi    ==
 == con esta inf. CS:EIP podemos localizar la instr. que fallo aunque no ==
 == siempre ;(                                                           ==
 == Posteriormente debe tratar las excepciones comunes como fallos de    ==
 == paginas, divisiones por cero, violaciones, etc.                      ==
 ======================================================================== */
PUBLICA void manejador(int nro, regs *r)
{
   mostrar("\nExcepcion: <%02d>  [%s]", nro, ayuda[nro].descr);
   mostrar("\nError    : <%08X>", r->codigo);
   mostrar("\nCR2      : <%08X>", CR2());
   
   /* muestra la descripcion de la excepcion que se produjo. */
   mostrar("\n[ AYUDA ]\n%s", ayuda[nro].ayuda);
   ver_regs(r);
   pausa();
}


      
/* ====================================================================== *
 == Completa la IDT con los vectores de la tabla.                        ==
 ======================================================================== */
PUBLICA void inic_excs()
{
   int i;
   
   mostrar("\nInicializando excepciones...");
   for(i=0; i<32; i++)
      const_idt( gate_table[i].vec_nr, (dir32)gate_table[i].fnc, gate_table[i].privilegios);
   
   const_idt( INT_SISTEMA, (dir32)int_sistema, INTR_PRIVILEGE );
}

