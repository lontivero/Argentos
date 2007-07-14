/* ====================================================================== *
 ==  Este archivo contiene las funciones necesarias para el manejo del   ==
 ==  teclado.                                                            ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <macros.h>
#include <debug.h>
#include <mensajes.h>
#include "teclado.h"
#include "terminal.h" 
#include "consola.h" 
#include "mapa_teclado.map"


#define FRECUENCIA_BIP    0x0533
#define DURACION_BIP           4
#define TAM_BUFFER_TECLADO    32

#define MEMCHECK_ADR       0x472  /* dir. para parar el chequeo de mem. despues de rebootear */
#define MEMCHECK_MAG      0x1234  /* nro. amgico para detener el cheque */


PRIVADA bool  enviar_teclado(unt8 dato);
PRIVADA void  est_LEDS();
PRIVADA bool  es_especial(unt8 t);
PRIVADA void  depurar(int tecla);
PRIVADA void  bip();
PRIVADA void  parar_bip();
PRIVADA void  rebootear();

PRIVADA tecla_rapida rapidas[NRO_TECLAS_RAPIDAS];
PRIVADA bool  emitiendo_bip;

PRIVADA unt8  estado;
PRIVADA unt8 *p_cabeza;
PRIVADA unt8 *p_cola;
PRIVADA int   cnt;
PRIVADA unt8  buf[TAM_BUFFER_TECLADO];


/* ====================================================================== *
 == enviar_teclado                                                       ==
 ==   prueba enviar un byte al controlador del teclado 5 veces.          ==
 ==   Devuelve TRUE si fue exitoso y FALSE si no lo fue.                 ==
 =========================================================================*/
PRIVADA bool  enviar_teclado(unt8 dato){
   int  err;
   err = 5;
   
   do{
      while(inp8(P_ESTADO) & BIT_BUFF_IN );
         outp8(P_DATO, dato);
         
      while(inp8(P_ESTADO) & BIT_BUFF_OUT);
         if(inp8(P_DATO)==0xFA) return TRUE;
         
   } while(err--);
   
   return FALSE;
}




/* ====================================================================== *
 ==                                                                      ==
 =========================================================================*/
PRIVADA void est_LEDS()
{
   unt8  leds;
   
   leds =  (estado & ESCROLL) | ((estado & ENUM) << 1) | ((estado & ECAPS) << 2);
   
   enviar_teclado(COM_LEDS);
   enviar_teclado(leds);
}



/* ====================================================================== *
 ==                                                                      ==
 =========================================================================*/
PRIVADA bool es_especial(unt8 t)
{
   switch(t) {
      case 0x36:
      case 0x2A: estado |=  ESHIFT;  break;
      case 0xB6:
      case 0xAA: estado &= ~ESHIFT;  break;
      case 0x1D: estado |=  ECTRL;   break;
      case 0x9D: estado &= ~ECTRL;   break;
      case 0x38: estado |=  EALT;    break;
      case 0xB8: estado &= ~EALT;    break;
      case 0x3A:
      case 0x45: break;
      case 0xBA: estado ^=  ECAPS;   break;
      case 0xC5: estado ^=  ENUM;    break;
      case 0xE0: estado ^=  ESCROLL; break;
      default  : return FALSE;
   }

//   est_LEDS();
   return TRUE;
}


/* ====================================================================== *
 == sacar_codigo                                                         ==
 =========================================================================*/
PRIVADA unt8 sacar_codigo()
{
   unt8 cod, val;
   cod = inp8(P_DATO);
   val = inp8(0x61);
   outp8(0x61, val | 0x80); 
   outp8(0x61, val);
   return cod;
}


/* ====================================================================== *
 == irq_teclado                                                          ==
 =========================================================================*/
PUBLICA bool irq_teclado(int irq, regs * r){
   unt8 t = sacar_codigo();
   
   /* si la tecla oprimida o liberada no es una tecla especial
    * como ALT, CONTROL o SHIFT, la ignoro.				
    */
   
   if(!es_especial(t) && !(t & 0x0200)){
   
      /* si el buffer no esta lleno, almaceno la tecla de lo
       * la ignoro y se pierde. Deberia enviar un beep ya veo
       */
      if(cnt < TAM_BUFFER_TECLADO){
   
         /*si llegamos al final del buffer comenzamos de nuevo
         * por el principio.
         */
         if(p_cabeza >= buf + TAM_BUFFER_TECLADO)
            p_cabeza  = buf;
      
         *p_cabeza++ = t;
         cnt++;
         term_actual->aviso = TRUE; 
      }else
         bip();
   
      /* si la tecla que aprete estaba precedida por un CTRL
       * me fijo para ver si no pertenece a alguna de las
       * combinaciones para depurar.
       */
      if(estado & ECTRL)
      depurar(mapa[t].ctrl);
   }
   
   return TRUE;
}


/* ====================================================================== *
 == irq_teclado                                                          ==
 =========================================================================*/
PUBLICA void leer_teclado(terminal *t)
{
   unt16 cod, tecla;
   char  b[3];

   /* mientras existan teclas en el buffer */
   while(cnt){
      cnt--;
      cod = *p_cola++;   /* tomo la primera tecla y avanzo*/
      
      /* si la cola esta fuera del buffer hago que apunte
       * al comienzo.
       */
      if(p_cola >= buf + TAM_BUFFER_TECLADO) p_cola = buf;
       
      /* si estaba apretada la tecla SHIFT o ALT o CTRL o 
       * una combinacion de estas, analizarlas y devolver
       * la tecla el caracter que corresponda.
       */
      if(estado & ESHIFT)      tecla = mapa[cod].shift;
      else if(estado & ECTRL)
        if(estado & EALT)      tecla = mapa[cod].ctrlalt;
        else                   tecla = mapa[cod].ctrl;
        else if(estado & EALT) tecla = mapa[cod].alt;
           else                tecla = mapa[cod].normal;
      
      /* si la tecla es una normal, la procesamos sin mas 
       * pero si es una tecla extendida la mandamos como
       * ESC[tecla
       */
      if(tecla < 0xFF){
         b[0] = tecla;
         term_procesar(t, b, 1);
      }
      else if(tecla >= HOME && tecla <= INSERTAR){
         b[0] = ESC;
         b[1] = '[';
         b[2] = tecla - HOME;
         term_procesar(t, b, 3);
      }else
      if(tecla >= AF1 && tecla <= AF10){
         cambiar_consola(tecla-AF1);
      }
   }  
}


/* ====================================================================== *
 ==                                                                      ==
 =========================================================================*/
PUBLICA void inic_teclado(){
   mostrar("\nInicializando teclado...");
   
   p_cabeza = buf;
   p_cola   = buf; 
   cnt      = 0;
   estado   = 0;
   emitiendo_bip = FALSE;
   
   est_man_irq(IRQ_TECLADO, irq_teclado);
}



/*===========================================================================
 == beep                                                                   ==
 ============================================================================*/
extern proc tabla_proc[];
PRIVADA void bip()
{
   mensaje m;
   
   if (emitiendo_bip) return;
   emitiendo_bip = TRUE;
   
   outp8(MODO_RELOJ, 0xB6);
   outp8(RELOJ2, FRECUENCIA_BIP);
   outp8(RELOJ2, (FRECUENCIA_BIP >> 8)); 
   desactivar();
   outp8(0x61, inp8(0x61) | 3);
   activar();
   
   m.SERVICIO = EST_ALARMA;
   m.TICKS    = DURACION_BIP;
   m.PROC     = T_TERM;
   m.FUNC     = (dir32)parar_bip;
   
   penviar(&tabla_proc[T_TERM], T_RELOJ, &m);
}


/*===========================================================================*
 ==  para_bip                                                               ==
 *===========================================================================*/
PRIVADA void parar_bip()
{
   desactivar();
   mostrar("\nbip apagado.......");
   outp8(0x61, inp8(0x61) & ~3);
   emitiendo_bip = FALSE;
   activar();
}



/*===========================================================================
 ==                              wreboot                                   == 
 ===========================================================================*/
PRIVADA void rebootear()
{
   unt16 magico = MEMCHECK_MAG;
   
   /* Ninguna interrupcion mas!!! */
   desactivar();
   
   /* Tell several tasks to stop. */
   /*  cons_stop();
       floppy_stop();
       clock_stop();
   */
   
   mostrar("\nAdios....");
   
   /* Detenemos el chequeo de memeoria tras el rebooteo */
   cp_mem((dir32)MEMCHECK_ADR, (dir32)&magico, sizeof(magico));
   
   /* Listo. resetarmos la PC */
   resetear();
}



PRIVADA void depurar(int tecla)
{
   desactivar();
   
   switch(tecla)
   {
      case C('p'):  ver_procs(); break;
      case C('e'):  int3();      break;
      case C('c'):  rebootear(); break;
/*      case 'd':  activar(); dir = obt_hex(); ver_mem(dir, 8); break; */
   }
   
   activar();
}
