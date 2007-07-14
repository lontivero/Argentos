/* ====================================================================== *
 ==  Este archivo contiene la funcion main() en C. Esta es llamada por   ==
 ==  mp.asm.                                                             ==
 ==  Basicamente inicializa:                                             ==
 ==    + la memoria                                                      ==
 ==    + las excepciones e interrupciones                                ==
 ==    + el pic                                                          ==
 ==    + y la tabla de procesos.                                         ==
 ==                                                                      ==
 ==   y gira la rueda (segun Tanembaum)                                  ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <macros.h>
#include <const.h>
#include "otros.h"



#define BCD2BIN(val)    ((10*((val>>4) & 0x0f)) + (val & 0x0f))
#define BASE_LSB        0x15
#define BASE_MSB        0x16
#define EXT_LSB         0x17
#define EXT_MSB         0x18


#define RSEGUNDOS       0x00
#define RMINUTOS        0x02
#define RHORA           0x04
#define RDIA_SEMANA     0x06
#define RDIA            0x07
#define RMES            0x08
#define RANO            0x09
#define RCENTURIA       0x32
#define RESTADOA        0x0A
#define RESTADOB        0x0B
#define RESTADOC        0x0C
#define RESTADOD        0x0D

/* Tipo de drive del floppy disk para A: y B:
 * El byte usa los bits 0:3 para A y los bits 4:7 para B
 * Los valores son:
 *    00h    no se encuentra un drive
 *    01h    floppy de 360k de dos caras
 *    02h    floppy de alta densidad (1.2 meg)
 *    03h    floppy de 720k
 *    04h    floppy de 1.44 megs
 *    05h    floppy de 2.88 megs
 */
#define TIPO_FLOPPY     0x10
#define DRIVE_A         0x00

PRIVADA int inCMOS(int p);
PRIVADA void outCMOS(int p, unt8 dato);


PRIVADA int inCMOS(int p)
{
   outp8(CMOS_PL, BYTE(p));
   return(BYTE(inp8(CMOS_PE)));
}

PRIVADA void outCMOS(int p, unt8 dato)
{
   outp8(CMOS_PL, BYTE(p));
   outp8(CMOS_PE, dato);
}


PUBLICA unt32 obt_memoria()
{
   unt32 mem_base;
   unt32 mem_ext;
   
   mem_base = inCMOS(BASE_LSB) | (inCMOS(BASE_MSB) << 8);
   mem_ext  = inCMOS(EXT_LSB)  | (inCMOS(EXT_MSB)  << 8);
   
   return _1KB_ * (mem_base + mem_ext);
}

PUBLICA void obt_fecha( unt8 *d, unt8 *mes, unt8 *a )
{
   *d   = BCD2BIN(inCMOS(RDIA));
   *mes = BCD2BIN(inCMOS(RMES));
   *a   = BCD2BIN(inCMOS(RANO));
}

PUBLICA void obt_hora(unt8 *h, unt8 *m, unt8 *s) 
{
   *s   = BCD2BIN(inCMOS(RSEGUNDOS));
   *m   = BCD2BIN(inCMOS(RMINUTOS));
   *h   = BCD2BIN(inCMOS(RHORA));
}

PUBLICA unt8 obt_dia()
{
   return BCD2BIN(inCMOS(RDIA_SEMANA));
}

PUBLICA unt8 obt_tipo_floppy(unt8 drv)
{
   unt8 tipo;
   tipo = inCMOS(TIPO_FLOPPY);
   
   if(drv == DRIVE_A)
      tipo >>= 4;
   else
      tipo &= 0x0f;
   
   return tipo;
}

