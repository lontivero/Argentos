#include <mem.h>
#include <mensajes.h>
#include "dram.h"
#include "driver.h"

#include "..\proc.h"
#include "consola.h"


PRIVADA void inic_dram();
PRIVADA dispositivo *preparar(int disp);
PRIVADA int operar(pid proc, operacion_es *es);
PRIVADA int abrir(driver *d, mensaje *m);

#define TAM_BLOQUE_DRAM    128
#define DRAM                 1

PRIVADA dispositivo geometria;  

PRIVADA driver dram_drv = {
   nombre,
   abrir,
   sin_op,
   sin_op,
   preparar,
   operar,
   sin_final,
   sin_limpieza,
   sin_geometria 
};


/* ========================================================================== *
 ==  t_dram                                                                  ==
 * ========================================================================== */
PUBLICA void t_dram()
{
   inic_dram();
   t_driver(&dram_drv);
}
   
/* ========================================================================== *
 ==  inic__dram                                                              ==
 * ========================================================================== */
PRIVADA void inic_dram()
{  
//   mensaje m;
   mostrar("\nInicializando disco RAM");

//   recibir(T_MEM_FISICA, &m);
   geometria.base = 0x40000; // m.DIRECCION; 
   geometria.tam  = 100*512; // m.CANTIDAD/TAM_BLOQUE_DRAM;
}

/* ========================================================================== *
 ==  t_dram                                                                  ==
 * ========================================================================== */
PRIVADA dispositivo *preparar(int disp)
{
   if(disp != DRAM) return NULL;

   return &geometria;
}
   

   
/*===========================================================================*
 *                                                                           *
 *===========================================================================*/
PRIVADA int operar(pid proc, operacion_es *es)
{
   unt32 cnt;
   dir32 dir;

   if(es->buf <= 0) return (es->cnt = E_INV);
   if(es->pos <  0) return (es->cnt = E_INV);

   if(es->pos >= geometria.tam)  return OK;

   cnt = es->cnt;
   if((es->pos + cnt) > geometria.tam)
      cnt = geometria.tam - es->pos;
  
   dir = geometria.base + es->pos;
   es->cnt -= cnt;

   if(cnt == 0) return OK;

   if (es->op == LEER_DISP) cp_mem(es->buf, dir, cnt);
   else                     cp_mem(dir, es->buf, cnt);

   return OK;
}


/*============================================================================*
 *                              m_do_open                                     *
 *============================================================================*/
PRIVADA int abrir(driver *d, mensaje *m)
{
   if(preparar(m->DISPOSITIVO) == NULL) return E_SIN_DISP;
   
   return OK;
}

