/* ====================================================================== *
 ==  Este archivo contiene las funciones necesarias para manejar las     ==
 ==  terminales.                                                         ==
 ==  Estas son:                                                          ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <mem.h>
#include <macros.h>
#include <sform.h>
#include "driver.h"
#include "consola.h"
#include "terminal.h" 
#include "teclado.h"
#include "..\proc.h"

#define FDE 0x80
#define NRO_TERMINALES  10
/* ====================================================================== *
 ==  funciones privadas                                                  ==
 =========================================================================*/
PRIVADA int term_abrir(struct driver *drv, mensaje *m);
PRIVADA int term_cerrar(struct driver *drv, mensaje *m);
PRIVADA dispositivo * term_preparar(int disp);
PRIVADA int  term_operar(pid p, operacion_es *es);
PRIVADA void term_leer();
PRIVADA void term_escribir();
PRIVADA void enviar_a_proceso(terminal * t);
PRIVADA void borrar_atras(terminal *t);
// PRIVADA void inic_terms();


PUBLICA terminal terminales[NRO_TERMINALES];
PRIVADA terminal * term_man;


PRIVADA dispositivo geometria;

/* ========================================================================== *
 ==  t_terminal                                                              ==
 * ========================================================================== */
PUBLICA void t_terminal()
{
/*PRIVADA*/ driver terminal_drv = {
   nombre,
   term_abrir,      /* abrir    */
   term_cerrar,     /* cerrar   */
   sin_op,          /* ioctrl   */ 
   term_preparar,   /* preparar */
   term_operar,     /* operar   */
   sin_final,       /* finalizar*/
   sin_limpieza,    /* limpieza */
   sin_geometria    /* geometria*/
};

   //inic_terms();
   t_driver(&terminal_drv);
}



/* ====================================================================== *
 ==  abre una terminal                                                   ==
 =========================================================================*/
PRIVADA int term_abrir(struct driver *drv, mensaje *m)
{
   int term;
   
   term = m->DISPOSITIVO;
   if(term_preparar(term) == NULL) return E_SIN_DISP;
   
   terminales[term].nro_aperturas++;
   return OK;
}


/* ====================================================================== *
 ==  cierra una terminal                                                 ==
 =========================================================================*/
PRIVADA int term_cerrar(struct driver *drv, mensaje *m)
{
   int term;
   
   term = m->DISPOSITIVO;
   if(term_preparar(term) == NULL) return E_SIN_DISP;
   
   terminales[term].nro_aperturas--;
   return OK;
}


/* ====================================================================== *
 ==  prepara el dispositivo                                              ==
 =========================================================================*/
PRIVADA dispositivo * term_preparar(int disp)
{
   if(disp < 0 || disp >= NRO_TERMINALES)
   {
      term_man = NULL;
      return NULL;
   }
   
   
   term_man = &terminales[disp];
   return &geometria;
}


/* ====================================================================== *
 ==  lee desde o escribe hacia una terminal                              ==
 =========================================================================*/
extern proc * tabla_proc;
PRIVADA int term_operar(pid p, operacion_es *es)
{
   unt32 cnt;
   
   if(es->buf <= 0) return (es->cnt = E_INV);
   
   cnt = es->cnt;
   
   if(cnt == 0) return OK;
   
   term_man->llamador = &tabla_proc[p];
   term_man->buffer   = es->buf;
   
   if(es->op == LEER_DISP){
      term_man->cnt_leer = cnt;
      term_leer();
   } else {
      term_man->cnt_escribir = cnt;
      term_escribir();
   }
   
   return OK;
}


PRIVADA void term_leer()
{
   if(term_man == NULL) return;
   
   do{
      enviar_a_proceso(term_man);
      manejar_avisos(term_man);
   }while(term_man->cnt_leer != 0);
}


PRIVADA void term_escribir()
{
   if(term_man == NULL) return;
   manejar_avisos(term_man);
   if(term_man->cnt_escribir == 0) return;
}



/* ====================================================================== *
 ==  Se acaba de oprimir una tecla asi que debemos procesarla            ==
 =========================================================================*/
PUBLICA void term_procesar(terminal *t, char *b, int c)
{
   int  i;
   char ch;
   
   for(i=0; i<c; i++){
      ch = b[i];
      
      if(t->escape){
         t->escape = FALSE;
      }
      else if(ch == '\\'){
         t->escape = TRUE;
         continue;
      }
      else if(ch == '\r') ch = '\n';
      else if(ch == '\b' /*RETRO*/)      borrar_atras(t);
//      else if(ch == '\v' /*RETRO_TODO*/) borrar_linea(t);
      
      if(ch == '\n' ) ch |= 0x80;
      
      *t->p_cabeza++ = ch;
      if(t->p_cabeza == t->buf_entrada + TAM_BUFFER_ENT)
         t->p_cabeza = t->buf_entrada;
      
      t->nro_entradas++;
      
      if(t->nro_entradas == TAM_BUFFER_ENT) enviar_a_proceso(t);
   }
}


PRIVADA void borrar_atras(terminal *t)
{
   if(t->nro_entradas == 0) return;
   
   if(t->p_cabeza == t->buf_entrada) t->p_cabeza = t->buf_entrada + TAM_BUFFER_ENT;
   t->nro_entradas--;
   t->p_cabeza--;
}


PRIVADA void enviar_a_proceso(terminal * t)
{
#define TAM_TEMP_BUF	64
   int cnt, ch;
   char buf[TAM_TEMP_BUF], *b;
   
   b = buf;
   
   /* proceso las entradas */
   while(t->cnt_leer && t->nro_entradas){
      t->nro_entradas--;
      ch = *t->p_cola;
      
      /* si no es fin de entrada */
      if(!(ch & FDE)) {
         *b = ch & 0x7f;	/* le quito la marca de FDE */
         t->cnt_leer--;
         
         /* si se lleno el buffer temporario, copio al usuario */
         if(++b == buf + TAM_TEMP_BUF) {
            cp_mem((dir32)t->buffer, (dir32)buf, TAM_TEMP_BUF);
            t->buffer += TAM_TEMP_BUF;
            b = buf;
         }
      }
      
      /* Quito el caracter de la cola */
      if(++t->p_cola == t->buf_entrada + TAM_BUFFER_ENT)
         t->p_cola = t->buf_entrada;
      
      /* si tiene la marca de FDE no proceso mas nada */
      if(ch & FDE) t->nro_entradas = 0;
   }
   
   /* envio al proceso de usr. todo el remanente */
   if(b > buf) {
      cnt = b - buf;
      cp_mem((dir32)t->buffer, (dir32)buf, cnt);
      t->buffer += cnt;
   }
}



/*===========================================================================*
 *                              handle_events                                *
 *===========================================================================*/
PUBLICA void manejar_avisos(terminal * t)
{
   do {
      t->aviso = FALSE;
      
      leer_teclado(t);
      escribir_consola(t);
   } while (t->aviso);
   
   /* envia los caracteres leidos al proceso */
   enviar_a_proceso(t);
}


/* ====================================================================== *
 ==  Inicializa la tabla de terminales                                   ==
 =========================================================================*/
extern cons consolas[];
/*PRIVADA*/ PUBLICA void inic_terms()
{   
//desactivar();
   /* Inicializo las terminales. y las consolasTodo a CERO */
   pt_mem((dir32)&terminales[0], 0, sizeof(terminal) * NRO_TERMINALES);
   pt_mem((dir32)&consolas[0],   0, sizeof(cons) * NRO_CONSOLAS);
   
   /* la terminal actual es la cero que pertenece al sistema */
   term_actual = &terminales[0];
   term_man    = term_actual;
   
   /* inicializo los punteros al buffer de entrada para el teclado */
   term_man->p_cabeza = term_man->buf_entrada;
   term_man->p_cola   = term_man->buf_entrada;
   
   /* inicializo la consola */ 
   inic_consola(term_actual);
//   inic_consola(term_actual+1);
   mostrar("Iniciando Terminales");
   
// activar();
}


