/* ===================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y        ==
 ==  prototipos de funciones necesarias el nucleo.                      ==
 =========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++      ==
 ==    ++ Por: Lucas Ontivero                                           ==
 ==    ++ email: lucasontivero@yahoo.com.ar                             ==
 ====================================================================== */

#ifndef _MSO_OTROS_H_
#define _MSO_OTROS_H_

#include <tipos.h>

/* ===================================================================== *
 == Constantes                                                          ==
 =======================================================================*/

/* =====================================================================.*
 == Estructuras                                                         ==
 ======================================================================= */

PUBLICA void obt_hora(unt8 *h, unt8 *m, unt8 *s);
PUBLICA void obt_fecha(unt8 *d, unt8 *m, unt8 *a);
PUBLICA unt8 obt_dia();
PUBLICA unt8 obt_tipo_floppy(unt8 drv);



#endif
