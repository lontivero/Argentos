#include "driver.h"

PRIVADA void inic_buffer();

/* ========================================================================= *
 ==  t_driver                                                               ==
 * ========================================================================= */
PUBLICA void t_driver(driver *d)
{
   int ret;
   unt16 origen;
   mensaje m;
   
   while (TRUE) {
      recibir(CUALQUIERA, &m);
      
      origen = m.m_origen;
      
//    if(m.m_origen != T_SA) continue;
      if(m.m_origen == T_RELOJ) continue;
      
      switch(m.SERVICIO) {
         case ABRIR_DISP :    ret = (*d->abrir_drv)(d, &m);  break;
         case CERRAR_DISP:    ret = (*d->cerrar_drv)(d, &m); break;
         case IOCTL_DISP :    ret = (*d->ioctl_drv)(d, &m);  break;
         case LEER_DISP  :
         case ESCRIBIR_DISP:  ret = operar_drv(d, &m);       break;
         default:             ret = E_SERVICIO;              break;
      }
      
      (*d->limpiar_drv)();
      
      m.RESPUESTA = ret;
      enviar(origen, &m); 
   }
}

/* ======================================================================= *
 ==  init_buffer                                                          ==
 * ======================================================================= */
PRIVADA void inic_buffer()
{}

	
/* ========================================================================== *
 ==  operar                                                                  == 
 * ========================================================================== */
PUBLICA int operar_drv(driver *d, mensaje *m)
{
   operacion_es es;
   int r;
   
   if (m->CANTIDAD <= 0) return E_INV;
   
   if ((*d->preparar_drv)(m->DISPOSITIVO) == NULL) return E_SIN_DISP;
   
   es.op  = m->SERVICIO;
   es.buf = m->DIRECCION;
   es.pos = m->POSICION;
   es.cnt = m->CANTIDAD;
   
   /* operar nunca puede ser NULL */
   r = (*d->operar_drv)(m->PROC, &es);
   
   if (r == OK) (*d->finalizar_drv)();
   
   r = es.cnt;
   return (r < 0 ? r : m->CANTIDAD - r);
}


/* ========================================================================= *
 ==  nombre                                                                 ==
 * ========================================================================= */
extern tarea *tabla_tarea;
PUBLICA char *nombre()
{
   return tabla_tarea[p_actual->nro].nombre;
}

/* ========================================================================== *
 ==  sin_op                                                                  ==
 * ========================================================================== */
PUBLICA int sin_op(driver *d, mensaje *m)
{
   switch (m->SERVICIO)
   {
      case ABRIR_DISP : return E_SIN_DISP;
      case CERRAR_DISP: return OK;
      case IOCTL_DISP : return E_SIN_IOCTL;
      default:          return E_ES;
   }
   
   return OK;
}

/* ========================================================================== *
 ==  sin_final                                                               ==
 * ========================================================================== */
PUBLICA int sin_final()
{
   return OK;
}
	
/* ========================================================================== *
 ==  sin_limpieza                                                            ==
 * ========================================================================== */
PUBLICA void sin_limpieza()
{}


/* ========================================================================== *
 ==  sin_geometria                                                            ==
 * ========================================================================== */
PUBLICA void sin_geometria(particion *p)
{
   p->base = p->tam = p->cils = p->cabs = p->secs = 0;
}

