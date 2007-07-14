/* ======================================================================*
 ==  Este archivo contiene las constantes, estructuras, macros y        ==
 ==  prototipos de funciones necesarias para el manejo de los procesos. ==
 =========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++      ==
 ==    ++ Por: Lucas Ontivero                                           ==
 ==    ++ email: lucasontivero@yahoo.com.ar                             ==
 =======================================================================*/

#include <mensajes.h>
#include <macros.h>
#include <mem.h>
#include "proc.h"
#include "reloj.h"
#include "mprot.h"
#include "memoria.h"
#include "disp\dram.h"
#include "disp\cdf.h"
#include "disp\consola.h"
#include "..\sa\sa.h"


PRIVADA void listo(proc * plisto);
PRIVADA void nolisto(proc * pnolisto);
PUBLICA void t_test();
PUBLICA void t_testa();
PUBLICA void lanzar_ints();

PUBLICA struct proc *cabeza_retenidos = NULL;
PUBLICA struct proc *cola_retenidos   = NULL;

EXTERNA char adentro;
PRIVADA bool cambiando;

PUBLICA tarea tabla_tarea[NRO_PROCESOS_TOTAL] =
{
   { "ociaosa   ",   PILA_OCIOSA,   t_ociosa     },
   { "terminal  ",   PILA_TERM,     t_terminal   },
   { "reloj     ",   PILA_RELOJ,    t_reloj      },
   { "mfisica   ",   PILA_MFISICA,  t_mem_fisica },
   { "disco_ram ",   PILA_DRAM,     t_dram       },
   { "floppy    ",   PILA_FLOPPY,   t_floppy     },
   { "test      ",   PILA_TEST,     t_test       }
};


proc  tabla_proc[NRO_PROCS];  /* tabla de procesos       */
unt8  pila_tarea[PILA_TOTAL];

typedef struct{
   proc * cabeza;
   proc * cola;
}  cola_proc;

#define PRIO_ALTA    0
#define PRIO_MEDIA   1
#define PRIO_BAJA    2
#define NRO_PRIORIDADES 3

cola_proc colas[NRO_PRIORIDADES] = { {NULL, NULL}, {NULL, NULL}, {NULL, NULL} };


#define cp_mensaje(d, o, oo)  \
   cp_mem((dir32)d, (dir32)o, sizeof(mensaje)); \
      d->quien = oo




PUBLICA int llamada_sistema(int func, int quien, mensaje * pm)
{
   proc *p_indice;
   int err;
   
   p_indice = p_actual;
   
   mostrar("\nrecibido: %d de %d: %s", func, quien, (char *)(pm->DIRECCION));
   
   if (func & ENVIAR) {
      err = penviar(p_indice, quien, pm);
      if (func == ENVIAR || err != OK)
         return err;
   }
   
   return precibir(p_indice, quien, pm);
}


/*=========================================================================
 == Conmuta las tareas.                                                  ==
 ======================================================================== */
PUBLICA void cambiar(bool girar)
{
   if( colas[PRIO_ALTA].cabeza == NULL )
      p_actual = &tabla_proc[T_OCIOSA];
   else
   {
      if(girar){
         colas[PRIO_ALTA].cola->prox = colas[PRIO_ALTA].cabeza;
         colas[PRIO_ALTA].cola = colas[PRIO_ALTA].cabeza;
         colas[PRIO_ALTA].cabeza = colas[PRIO_ALTA].cabeza->prox;
         colas[PRIO_ALTA].cola->prox = NULL;
      }
   
      p_actual = colas[PRIO_ALTA].cabeza;
   }
   
   if( p_actual == NULL )
      p_actual = &tabla_proc[T_OCIOSA];

/*
   __asm__ __volatile__ (
   "ljmp %0"
   :
   : "m" (*(   ((char *)&p_actual->p_tss.cs)   -4))
   : "memory"
   );
*/
}




/*=========================================================================
 == construye las tareas del sistema.                                    ==
 == (Llena la tabla de procesos)                                         ==
 ======================================================================== */
PUBLICA void inic_procs()
{
   int i;
   struct proc *p_proc;
   unt8 *pila;
   pila = pila_tarea;
   p_actual = NULL;
   tss * p_tss;
   
   /* Limpiamos la tabla de procesos  */
   for(i=0; i<NRO_PROCS; i++)
      tabla_proc[i].estado = LIBRE;
   
   
   
   for(i = 0, p_proc = DIR_INI_PROC; p_proc < DIR_ULT_PROC; p_proc++, i++)
   {
      pt_mem((dir32)p_proc, 0, sizeof(proc));
   
      p_proc->nro = i;
      cp_cad(p_proc->nombre, tabla_tarea[i].nombre);
   
   
      p_tss = &p_proc->p_tss;
      p_tss->esp0   = 0x200 * (i+1);
      p_tss->ss0    = SEL_DATO_LINEAL;
      p_tss->eip    = (dir32)tabla_tarea[i].rutina;
      p_tss->eflags = FLAGS_INICIAL;
      p_tss->esp    = (dir32)(pila += tabla_tarea[i].tam_pila);
      p_tss->ldt    = p_proc->selector_ldt;
   
      /* construirmos el descriptor para el TSS */
      asignar_segmentos(p_proc);
      listo(p_proc);
   }

   p_actual = &tabla_proc[T_OCIOSA];
//   cambiar(TRUE);
//   nolisto(&tabla_proc[T_OCIOSA]);
}


PUBLICA int penviar(proc *origen, pid destino, mensaje * pm)
{
   proc * p_destino;
   proc * p_indice;

   /*
    * Nos fijamos si los procesos origen y destino se estan
    * enviando mutuamente o si el destino envia a un tercero
    * que envia al origen y este a su vez al destino.
    * En sintesis todos nos estamos esperando y nadie puede
    * dar el primer paso.
    */
   p_destino= &tabla_proc[destino];
   if(p_destino->estado & ENVIANDO)
   {
      p_indice = &tabla_proc[p_destino->enviando_a];
      while(TRUE)
      {
         if(p_indice == origen){    /* se entan eviando mutuamente? */
            mostrar("bloque mutuo por %d a %d", origen->nro, destino);
            return E_BLOQUEO_MUTUO;
         }
         /* le esta enviando a un proceso que a su vez esta enviando */
         if(p_indice->estado & ENVIANDO)
            p_indice = &tabla_proc[p_indice->enviando_a];
         else break;
      }
   }
   
   /*
   * Ahora nos fijamos si el destino espera a un proceso
   * CUALQUIERA o al proceso que envia.
   */
   
   /* el destino esta esperando este mensaje */
   if((p_destino->estado & (RECIBIENDO|ENVIANDO))==RECIBIENDO)
   {
      if(p_destino->recibiendo_de == CUALQUIERA || p_destino->recibiendo_de == origen->nro)
      {
         cp_mensaje(p_destino->pmensaje, pm, origen->nro);
         p_destino->estado &= ~RECIBIENDO;
         if(p_destino->estado==LISTO) listo(p_destino);
         return OK;
      }
   }
   
   /*
   *  El destino no esta esperando este mensaje asi que
   *  ponemos al proceso origen en la cola
   */
   p_indice = p_destino->primer_llamador;
   if( p_indice == NULL)
      p_destino->primer_llamador = origen;
   else {
      while(p_indice->prox_llamador != NULL)
         p_indice = p_indice->prox_llamador;
   
      p_indice->prox_llamador = origen;
   }
   
   origen->prox_llamador  = NULL;
   
   origen->pmensaje   = pm;
   origen->enviando_a = destino;
   origen->estado    |= ENVIANDO;
   if(origen->estado == ENVIANDO) nolisto(origen);
   
   return OK;
}


PUBLICA int precibir(proc * llamador, pid origen, mensaje * pm)
{
   proc *p_indice;
   proc *p_indice_anterior = NULL;
   
   /*
   * Nos fijamos si el mensaje que estamos esperando
   * esta en la cola.
   */
   if(!(llamador->estado & ENVIANDO))
   {
      p_indice = llamador->primer_llamador;
      while(p_indice != NULL)
      {
         if(origen == CUALQUIERA || origen == p_indice->nro)
         {
            cp_mensaje(pm, p_indice->pmensaje, p_indice->nro);
            
            /* saco al proceso de la cola */
            if(p_indice == llamador->primer_llamador)
               llamador->primer_llamador = p_indice->prox_llamador;
            else
               if(p_indice_anterior != NULL)
                  p_indice_anterior->prox_llamador = p_indice->prox_llamador;
               else
                  mostrar("error!! precibir\n");
            
            
            p_indice->estado &= ~ENVIANDO;
            if(p_indice->estado==LISTO) listo(p_indice);
            
            return OK;
         }
         p_indice_anterior = p_indice;
         p_indice=p_indice->prox_llamador;
      
      }  /*endwhile */
      
      /* Si no esta. Nos fijamos si espera un INT. */
      if(llamador->esperando_int && (origen == HARDWARE || origen == CUALQUIERA))
      {
         pm->m_origen = HARDWARE;
         pm->codigo   = INTERRUPCION;
         llamador->esperando_int = FALSE;
         return OK;
      }
   
   }  /*endif*/
   
   /*
   * Si no esta el mensaje.
   */
   llamador->recibiendo_de = origen;
   llamador->pmensaje      = pm;
   llamador->estado       |= RECIBIENDO;
   if(llamador->estado == RECIBIENDO)
      nolisto(llamador);   /* si no estaba bloqueado, bloquealo */
   
   return OK;
}



PRIVADA void listo(proc * plisto)
{
   if(colas[PRIO_ALTA].cabeza != NULL)
      colas[PRIO_ALTA].cola->prox = plisto;
   else
      colas[PRIO_ALTA].cabeza = plisto;
   
   plisto->prox = NULL;
   colas[PRIO_ALTA].cola = plisto;
}


PRIVADA void nolisto(proc * pnolisto)
{
   proc * p_indice = colas[PRIO_ALTA].cabeza;
   if(p_indice == NULL) return;
   if(p_indice == pnolisto)
   {
      colas[PRIO_ALTA].cabeza = colas[PRIO_ALTA].cabeza->prox;
      if(pnolisto == p_actual) cambiar(FALSE);
      return;
   }
   
   while(p_indice->prox != pnolisto)
   {
      p_indice = p_indice->prox;
      if(p_indice==NULL) return;
   }
   
   p_indice->prox = p_indice->prox->prox;
   if(pnolisto == colas[PRIO_ALTA].cola)
      colas[PRIO_ALTA].cola = p_indice;
}


/*=========================================================================
 ==                                                                      ==
 ======================================================================== */
PUBLICA void interrupcion(pid tarea)
{
   proc *p_proc = &tabla_proc[tarea];
   
   /* Para resolver problemas con las ints. reentrantes
    * si esta llamada compite con otra, se la pone en la cola 'retenida' para
    * ser disparada en la proxima sin competencia.
    * La competencia se da cuando:
    * (1) reentrando > 0:
    *     llamada desde un manejador de ints. anidado. Un manejador de ints.
    *     anterior esta dentro de interrupcion() o sys_call().
    * (3) cambiando != 0:
    *     algun cambio de proceso se esta realizando. Tipicamente cambiar()
    *     desde el t_reloj.  Un manejador de ints. podria llamar a interrupcion
    *     y pasar es test de 'reentrando'
    */
   if(adentro != 0 || cambiando == TRUE) {
      desactivar();
      if (p_proc->int_retenida == FALSE) {
         p_proc->int_retenida = TRUE;
         if (cabeza_retenidos == NULL)
            cabeza_retenidos = p_proc;
         else
            cola_retenidos->p_prox_retenido = p_proc;
   
         cola_retenidos = p_proc;
         p_proc->p_prox_retenido = NULL;
      }
      activar();
      return;
   }
   
   
   /* Si la tarea no esta esperando una INT. Guardamos este estado. */
   if( (p_proc->estado & (RECIBIENDO | ENVIANDO)) != RECIBIENDO ||
   !(p_proc->recibiendo_de == CUALQUIERA || p_proc->recibiendo_de == HARDWARE)) {
      p_proc->esperando_int = TRUE;
      return;
   }
   
   /* Si la tarea esta esperando una INT. Le enviamos un mensaje para
   *  desbloquearla.
   */
   p_proc->pmensaje->m_origen = HARDWARE;
   p_proc->pmensaje->codigo   = INTERRUPCION;
   p_proc->estado &= ~RECIBIENDO;
   p_proc->esperando_int = FALSE;
   
   /* Ponemos la tarea en la cola de ejecucion */
   listo(p_proc);
}


/*==========================================================================*
 *                              unhold                                      *
 *==========================================================================*/
PUBLICA void lanzar_ints()
{
   struct proc *p_proc; 

   asm("int $0x3;");

   if (cambiando) return;
   p_proc = cabeza_retenidos;
   
   do {
      cabeza_retenidos = p_proc->p_prox_retenido;
      if ( cabeza_retenidos == NULL) cola_retenidos = NULL;
      p_proc->int_retenida = FALSE;
      /*unlock();*/               /* reduce latency; held queue may change! */
      interrupcion(p_proc->nro);
      /*lock(); */                /* protect the held queue again */
   
      p_proc = cabeza_retenidos;
   }
   while ( p_proc != NULL);
}









/*
 * Que hace la PC cunado no hace nada? Esto!!!.
 */
PUBLICA void t_ociosa()
{
   for(;;);     /* for ever */
}


PUBLICA void t_test()
{
   mensaje m;
   int i;

   for(;;){
      m.SERVICIO  = OBT_FECHAHORA;
      enviar(T_RELOJ, &m);
      recibir(T_RELOJ, &m);
      for(i=0; i<0xffff; i++);
 
      mostrar("gil"); /*..\n); %02d:%02d:%02d \t", AHORAS(m.HORA), AMINUTOS(m.HORA), ASEGUNDOS(m.HORA));*/
/*     mostrar("%02d/%02d/%d", ADIAS(m.FECHA), AMESES(m.FECHA), AANOS(m.FECHA)); */

  }
}
