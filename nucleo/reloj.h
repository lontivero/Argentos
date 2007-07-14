/* ====================================================================== *
 ==  Este archivo contiene las funciones nacesarias para manejar las     ==
 ==  irqs.                                                               ==
 ==  Estas son:                                                          ==
 ==    + est_man_irq .establece una funcion como manejador de la irq     ==
 ==             especificada y la activa.                                ==
 ==    + inic_irq    .llena tabla de irqs con una funcion tonta.         ==
 ==    + aviso_irq   .la funcion antes mencionada                        ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#ifndef _MSO_RELOJ_H_
#define _MSO_RELOJ_H_

#include <tipos.h>
#include "irq.h"
#include "proc.h"




#define C_HORA          12
#define C_MINUTO         6
#define C_DIA           16
#define C_MES           12


#define HORA_A_BITS(h, m, s)  (unt32)((s) | (m << C_MINUTO) | (h << C_HORA))
#define FECHA_A_BITS(d, m, a) (unt32)(ANO_BASE(a)  | (m << C_MES) | (d << C_DIA))

#define ASEGUNDOS(h)          (h & 0x3F)
#define AMINUTOS(h)           ((h >> C_MINUTO) & 0x3F)
#define AHORAS(h)             ((h >> C_HORA)   & 0x1F)

#define AANOS(f)              (f & 0x7FF)
#define AMESES(f)             ((f >> C_MES) & 0x0F)
#define ADIAS(f)              ((f >> C_DIA) & 0x1F)

#define ALARMA_LIBRE   -1

/* ====================================================================== *
 ==  tipos                                                               ==
 ======================================================================== */
typedef struct alarma
{
   int    ticks;
   void  (*func)();
   struct alarma * prox;
}  alarma;

/* ====================================================================== *
 ==  Funciones                                                           ==
 ======================================================================== */
PUBLICA bool irq_reloj(int irq, regs * r);
PUBLICA void inic_reloj();
PUBLICA void t_reloj();

#endif
