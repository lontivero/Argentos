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

#include <mensajes.h>
#include <macros.h>
#include "reloj.h"
#include "otros.h"
#include "disp\consola.h"

/* ====================================================================== *
 ==  Configura la frecuencia de las interrupciones del reloj             ==
 ======================================================================== */
       
#define HZ                 60
#define MILLISEC          100
#define TICKS_CAMBIO      6*(MILLISEC*HZ/1000)

#define ANO_BASE(anno)    (2000+anno)
#define ES_BISIESTO(anno) ((ANO_BASE(anno)%4) == 0)


PRIVADA void avisar_inic_reloj();
PRIVADA int  est_alarma(mensaje *m);
PRIVADA void est_frecuencia(unt32 frec);
PRIVADA int  est_fecha_hora(mensaje *mr);
PRIVADA int  obt_fecha_hora(mensaje *mr);

int ticks        = 0;
int ticks_cambio = 0;

#define NRO_ALARMAS  (3 * NRO_PROCS)
alarma *cabeza = NULL;
alarma alarmas[NRO_ALARMAS];


char *dias[]  = { "Domingo",    "Lunes",     "Martes",    "Miercoles",
                  "Jueves",     "Viernes",   "Sabado"  };

char *meses[] = { "Enero",      "Frebrero",  "Marzo",     "Abril", 
                  "Mayo",       "Junio",     "Julio",     "Agosto", 
                  "Septiembre", "Octubre",   "Noviembre", "Diciembre" };


/* ====================================================================== *
 ==  Configura la frecuencia de las interrupciones del reloj             ==
 ======================================================================== */
PUBLICA void t_reloj()
{
   int err;
   mensaje  m_reloj;

   avisar_inic_reloj();

   while(TRUE)
   {
      recibir(CUALQUIERA, &m_reloj);
      err = OK;

      switch(m_reloj.SERVICIO)
      {
         case OBT_FECHAHORA:
              err = obt_fecha_hora(&m_reloj);
              break;
         case EST_FECHAHORA:
              err = est_fecha_hora(&m_reloj);
              break;
         case EST_ALARMA  :
              err = est_alarma(&m_reloj);
              break;
         default:
              err = E_PARAMETRO;
      }
      
      m_reloj.RESPUESTA = err;
      enviar(m_reloj.PROC, &m_reloj);
   }
}


/* ====================================================================== *
 ==  Agrega una alarma a la lista ordenada de alarmas                    ==
 ======================================================================== */
PRIVADA int est_alarma(mensaje *m)
{
   int i;
   alarma * a_indice;
   alarma * a_libre;
   
   if(!es_tarea(m->PROC)) return E_OP_INV;
   
   for(i=0; i< NRO_PROCS; i++)
   if(alarmas[i].ticks == ALARMA_LIBRE) break;
   
   if(i==NRO_PROCS) return -1; 
   
   a_libre = &alarmas[i];
   a_libre->ticks = ticks + m->TICKS;
   a_libre->func  = (void (*)())m->FUNC;
   
   if(cabeza == NULL)
   {
      cabeza = a_libre;
      a_libre->prox = NULL; 
   }
   else if(m->TICKS < cabeza->ticks)
   {
      a_libre->prox = cabeza;
      cabeza = a_libre;
   }
   else
   {
      a_indice = cabeza->prox;
 
      while(a_indice->prox != NULL)
         if(m->TICKS < a_indice->prox->ticks)
         {
            a_libre->prox  = a_indice->prox;
            a_indice->prox = a_libre;
            break;
         }
      
      a_indice->prox = a_libre;
      a_libre->prox = NULL; 
   }

   return OK;
}


/* ====================================================================== *
 ==  Configura la frecuencia de las interrupciones del reloj             ==
 ======================================================================== */
PUBLICA void inic_reloj()
{
   int i;

   /* limpiamos las alarmas */
   for(i=0; i< NRO_PROCS; i++)
      alarmas[i].ticks = ALARMA_LIBRE;

   cabeza = NULL;
   
   est_frecuencia(HZ);  
   est_man_irq(IRQ_RELOJ, irq_reloj);
}


/* ====================================================================== *
 ==  manejador del reloj                                                 ==
 ======================================================================== */
extern proc tabla_proc[];
PUBLICA bool irq_reloj(int irq, regs * r)
{
   void (*f)();

   ticks++;
   ticks_cambio++;

   if(cabeza != NULL)
      if(ticks >= cabeza->ticks)
      {
         f = cabeza->func;
         cabeza->ticks = ALARMA_LIBRE;
         cabeza = cabeza->prox;
         (*f)();
      }

   if(ticks_cambio>=TICKS_CAMBIO)
   {
      ticks_cambio = 0;
      cambiar(TRUE);
   }
   
   return TRUE;
}


/* ====================================================================== *
 ==  Establece la frecuencia del reloj												 ==
 ======================================================================== */
PRIVADA void est_frecuencia(unt32 frec)
{
   unt16 div;

   if (frec < 20) frec = 75;

   div = FREQ_RELOJ / frec;

   outp8(MODO_RELOJ, ONDA_CUADRADA);
   outp8(RELOJ0, BYTE(div));
   outp8(RELOJ0, div >> 8);
}



/* ====================================================================== *
 ==  Devuelve la fecha y la hora del CMOS                                ==
 ======================================================================== */
PRIVADA int obt_fecha_hora(mensaje *mr)
{
   unt8 s, m, h, d, mes, a, ds;

   obt_fecha(&d, &mes, &a);
   obt_hora(&h, &m, &s);
   ds = obt_dia();

   /*
    *    6 bits para los segundos       5 bits para el DIA
    * +  6 bits para los minutos     +  4 bits para el MES
    *    5 bits para las horas         11 bits para el ANO
    * ---------------------------    ----------------------
    *    17 bits ( HORA )              20 bits ( FECHA )
    */

   mr->HORA  = HORA_A_BITS(h, m, s);
   mr->FECHA = FECHA_A_BITS(d, mes, a);
   mr->DIA_SEMANA = ds;
   return OK;
}



/* ====================================================================== *
 ==  Establece la fecha y la hora del CMOS                               ==
 ======================================================================== */
PRIVADA int est_fecha_hora(mensaje *m)
{
   return OK;
}


PRIVADA void avisar_inic_reloj()
{
   unt8 s, m, h, d, mes, a, ds;

   obt_fecha(&d, &mes, &a);
   obt_hora(&h, &m, &s);
   ds = obt_dia();

   mostrar("\nInicializando reloj a %d Hz.", HZ);
   mostrar("\n\tHora : %02d:%02d:%02d:", h, m, s);
   mostrar("\n\tFecha: %s, %d de %s de %d", dias[ds-1], d, meses[mes-1], 2000+a);
   mostrar("\n\t%d alarmas", NRO_ALARMAS);
}

