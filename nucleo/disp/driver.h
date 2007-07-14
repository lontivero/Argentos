/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias la parte dependiente de las E/S  ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#ifndef _MSO_DRIVER_H_
#define _MSO_DRIVER_H_

#include <tipos.h>
#include <mensajes.h>
#include "..\proc.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define LEER   (unt8)0x01
#define ESCRIBIR  (unt8)0x02


/* ====================================================================== *
 == Estructuras                                                          ==
 ======================================================================== */

typedef struct{
   unt8  op;
   unt32 pos;
   dir32 buf;
   unt32 cnt;
}  operacion_es;

typedef struct{
   dir32 base;
   unt32 tam;
}  dispositivo;

typedef struct {
   dir32 base;
   unt32 tam;
   unt16 cils;
   unt16 cabs;
   unt16 secs;
}  particion;


typedef struct driver{
   char * (*nombre_drv)();
   int    (*abrir_drv) (struct driver *drv, mensaje *m);
   int    (*cerrar_drv)(struct driver *drv, mensaje *m);
   int    (*ioctl_drv) (struct driver *drv, mensaje *m);
   dispositivo * (*preparar_drv)(int disp);
   int    (*operar_drv)(pid proc, operacion_es *ioreq);
   int    (*finalizar_drv)();
   void   (*limpiar_drv)();
   void   (*geometria_drv)(particion *p);
}  driver; 



/* ====================================================================== *
 == Funciones exportables                                                ==
 ======================================================================== */
PUBLICA void   t_driver(driver *drv);
PUBLICA int    operar_drv(driver *d, mensaje *m);
PUBLICA char  *nombre();
PUBLICA int    sin_op(driver *d, mensaje *m);
PUBLICA int    sin_final();
PUBLICA void   sin_limpieza();
PUBLICA void   sin_geometria(particion *p);



#endif
