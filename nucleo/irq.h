/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo de las irqs.      ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#ifndef _MSO_IRQ_H_
#define _MSO_IRQ_H_

#include <const.h>
#include <tipos.h>

/* ====================================================================== *
 == Tipos                                                                ==
 ======================================================================== */

/* ====================================================================== *
 == prototipos de funciones                                              ==
 ======================================================================== */
PUBLICA void inic_pic();
PUBLICA void inic_irqs();
PUBLICA void est_man_irq(int irq, man_irq man);
PUBLICA void activar_irq(int irq);
PUBLICA void desactivar_irq(int irq);

/* ====================================================================== *
 == Variables externas                                                   ==
 ======================================================================== */

#endif
