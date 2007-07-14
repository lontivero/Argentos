/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo del 386 en modo   ==
 ==  protegido.                                                          ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#ifndef _MSO_MPROT_H_
#define _MSO_MPROT_H_

#include <tipos.h>
#include <const.h>
#include "proc.h"


#define CR0_PG           0x80000000
#define CR0_TS           0x00000008


/* ====================================================================== *
 == Manejamos los tipos como un (unt16) con el siguiente formato.        ==
 ==                                                                      ==
 == 15   14   13   12   11   10    9    8    7    6   5  4  3  2 1  0    ==
 == [ G ][ D ][ 0 ][AVL][ 0 ][ 0 ][ 0 ][ 0 ][ P ][ DPL ][   TIPO    ]    ==
 ==                                                                      ==
 == Las constantes que siguen son para este formato y no para el de los  ==
 == descriptores del 386.                                                ==
 ======================================================================== */
#define GRANULAR         0x8000
#define DEFAULT          0x40
#define GRANDE           0x4000
#define OP_32B           0x0400
#define OP_16B           0x0000
#define AUSENTE          0x00
#define ACCEDIDO         0x01
#define ESCRIBIBLE       0x02
#define LEIBLE           0x02
#define EXPANDIBLE       0x04
#define APLICACION       0x10
#define SISTEMA          0x00
#define CONFORMING       0x04
#define PRESENTE         0x80
#define DPL_0            0x00
#define DPL_1            0x20
#define DPL_2            0x40
#define DPL_3            0x60
#define DPL_MASCARA      0x60

#define TIPO_LDT_DESCR   0x02
//#define TIPO_CALL_GATE 0x04
#define TIPO_TASK_GATE   0x05
#define TIPO_TSS_DESCR   0x09
#define TIPO_BUSYTSS_DESCR   0x0B
#define TIPO_INTR_GATE   0x0E
#define TIPO_TRAP_GATE   0x0F
#define TIPO_CALL_GATE   0x0C
                            
#define TIPO_CODE_DESCR  0x18
#define TIPO_DATA_DESCR  0x10




#define PRIV_KERNEL      DPL_0
#define PRIV_SERVICIO    DPL_1
#define PRIV_USUARIO     DPL_3
                         
#define RPL_0            0x0000
#define RPL_1            0x0001
#define RPL_2            0x0002
#define RPL_3            0x0003

#define SEG_CODIGO       GRANULAR   | PRESENTE   | TIPO_CODE_DESCR | LEIBLE
#define SEG_DATOS        GRANULAR   | PRESENTE   | TIPO_DATA_DESCR | ESCRIBIBLE 
#define SEG_TSS          PRESENTE   | TIPO_TSS_DESCR
#define SEG_LDT          PRESENTE   | TIPO_LDT_DESCR


#define NRO_DESCRIPTORES_GDT   1024
#define NRO_DESCRIPTORES_LDT   1024
#define NRO_DESCRIPTORES_IDT    256

/*
 * Para los selectores.
 */
#define ES_LOCAL        0x0004
#define ES_GLOBAL       0x0000



#define TAM_DESCR       sizeof(descriptor)


/*
 * Indices de los selectores predefinidos.
 */
#define IDX_NULO             0
#define IDX_CODIGO_LINEAL    1
#define IDX_DATO_LINEAL      2
#define IDX_GDT              3
#define IDX_IDT              4
#define IDX_CODIGO_NUCLEO    5
#define IDX_DATO_NUCLEO      5
#define IDX_NUCLEO_TSS       7
#define IDX_VIDEO            8
#define IDX_TSS              9
#define PRIMER_IDX_LIBRE    10



/*
 * Selectores predefinidos.
 */
#define SEL_NULO           0x00
#define SEL_CODIGO_LINEAL  0x08
#define SEL_DATO_LINEAL    0x10
#define SEL_GDT            0x18
#define SEL_IDT            0x20
#define SEL_CODIGO_NUCLEO  0x28
#define SEL_DATO_NUCLEO    0x30
#define SEL_NUCLEO_TSS     0x38
#define SEL_VIDEO          0x40
#define SEL_TSS            0x48


#define IDX_CS_LDT           0
#define IDX_DS_LDT           1
#define IDX_SS_LDT           2


/* ====================================================================== *
 == Estructuras para el 386 en modo protegido.                           ==
 ======================================================================== */


/* gate */
typedef struct
{
   unt16 direccion00_15 __attribute__((packed));
   selector sel         __attribute__((packed));
   unt8  nada           __attribute__((packed));
   unt8  tipo           __attribute__((packed));
   unt16 direccion16_31 __attribute__((packed));
}  __attribute__((packed)) gate;


/* pseudo_descriptor */
typedef struct 
{
   unt16 limite         __attribute__((packed));
   dir32 direccion      __attribute__((packed));
}  __attribute__((packed)) pseudo;




/* ====================================================================== *
 == prototipo de funciones                                               ==
 ======================================================================== */
PUBLICA void const_idt(unt16 nro, dir32 direccion, unt8 tipo);
PUBLICA bool const_descr(descriptor * descr, dir32 base, unt32 limite, unt16 tipo);
PUBLICA void inic_segcodigo(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios);
PUBLICA void inic_segdatos(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios);
PUBLICA void dump_descr(selector sel);
PUBLICA void inic_mp();
PUBLICA void resetear();

#endif
