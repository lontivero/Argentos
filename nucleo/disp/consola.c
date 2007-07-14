/* ====================================================================== *
 ==  Este archivo contiene las funciones necesarias para manejar una     ==
 ==  consola.                                                            ==
 ==  Estas son:                                                          ==
 ==    + est_pos_cursos    .establece la posicion del cursor             ==
 ==    + subir (scroll)    .sube el texto una linea                      ==
 ==    + limpiar  (clrscr) .limpia la pantalla                           ==
 ==    + color             .establece el atributo del texto              ==
 ==    + mostrat           .despliega texto formateado en la consola     ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

/* Hacer que borre la linea cuando tiro un cero */

#include <mem.h>
#include <macros.h>
#include <sform.h>
#include <mensajes.h>
#include "consola.h"
#include "terminal.h"
#include "..\mprot.h"



#define P_6845_IND      0x3D4
#define P_6845_DAT      0x3D5

#define ORIGEN             12
#define CURSOR             14


PUBLICA cons consolas[NRO_CONSOLAS];
PRIVADA cons * cons_actual;
PRIVADA char * video = (char *)MEM_VIDEO;

/* ====================================================================== *
 ==  funciones privadas                                                  ==
 =========================================================================*/
PRIVADA void subir(cons * consola);
PRIVADA void CRTC(int reg, unt16 val);
PRIVADA void flush(cons * consola);
PRIVADA void out_char(cons * consola, char c);
PRIVADA void cp_video(cons *consola);


PUBLICA void escribir_consola(terminal *t)
{
/* 80 caracteres o una linea del monitor */
#define TAM_BUF_TEMP	80
   int  cnt;                       /* cant. a escribir o TAM_BUF_TEMP */
   char buf[TAM_BUF_TEMP], *b;     /* buffer temporario */
   cons *consola = t->disp;

   /* Mientras exista algo para mostrar */
   while(t->cnt_escribir){
      cnt = t->cnt_escribir;
 
      /* si la cant. a escribir supera el tamano del buffer temporario */
      if(cnt > TAM_BUF_TEMP) cnt = TAM_BUF_TEMP;
 
      /* copiamos los datos al buffer temporario */
      cp_mem((dir32)buf, (dir32)t->buffer, cnt);
      b = buf;
 
      t->buffer       += cnt;      /* avanza en el buffer */
      t->cnt_escribir -= cnt;      /* decuenta los caracteres que se escribiran */
 
      /* mientras existan caracteres en el buffer temporario los sacamos */
      do{
      
         /* si es un caracter especial o el buffer de la consola esta lleno
          * hay que sacarlo directamente sin mas tramites.
          */
         if(*b < ' ' || consola->cnt_ram == TAM_BUFFER_VIDEO)
            out_char(consola, *b++);
 
         /* de lo contrario hay que ponerlos en el buffer de la consola con los
          * atributos correspondientes y mover el cursor.
          */
         else{
            consola->mem_ram[consola->cnt_ram++] = consola->atrib | (*b++ & 0xff);
            consola->posx++;
         }
      }while(--cnt != 0);
   }

   /* enviar cualquier cosa que quede en el buffer a la pantalla */
   flush(consola);
}



/*===========================================================================*
 *                              out_char                                     *
 *===========================================================================*/
PRIVADA void out_char(cons * consola, char c)
{
   int nueva_pos;
   /* Output a character on the console.  Check for escape sequences first. */
   /*   if (cons->c_esc_state > 0) {
           parse_escape(cons, c);
           return;
   }
 */
 
   switch(c) {
      case 0:
         return;
      
      case '\b':                   /* backspace */
        consola->posx--;           /* vuelvo una posicion en x */
        if(consola->posx < 0){     /* si estoy en la col. 0 */
           consola->posy--;        /* vuelvo una posicion en y */
           if(consola->posy >= 0)  /* si no estoy en la fila. 0 */
              consola->posx += NRO_COLUMNAS_VIDEO;  /* me voy al ultimo */
        }
      
      flush(consola);
      return;
      
      case '\n':              /* line feed */
         if(consola->cnt_ram == TAM_BUFFER_VIDEO) flush(consola);
         consola->mem_ram[consola->cnt_ram++] = consola->atrib;
         consola->posx = 0;
      
      case 013:               /* CTRL-K */
      case 014:               /* CTRL-L */
         if(consola->posy == NRO_FILAS_VIDEO-1){
            subir(consola);
         }else{
            consola->posy++;
         }
         flush(consola);
         return;
      
      case '\r':              /* carriage return */
         consola->posx = 0;
         flush(consola);
         return;
         
      case '\t':              /* tab */
         nueva_pos = (consola->posx + TAB_TAM) & ~TAB_MSK;
         while(consola->posx < nueva_pos) out_char(consola, ' ');
      
 /*
         if(consola->posx > NRO_COLUMNAS_VIDEO) {
            consola->posx -= NRO_COLUMNAS_VIDEO;
            if(consola->posy == NRO_FILAS_VIDEO-1){
               subir(consola);
            }else{
               consola->posy++;
            }
         }
         flush(consola);
 */
         return;
 
 #if 0
      case 27:                /* ESC - comienza una secuencia de escape */
         flush(consola);      /* imprime lo que tenga el buffer de salida */
         consola->c_esc_state = 1;  /* marca el; escape */
         return;
 #endif
   
      default:                /* si es un caracter comun va al buffer*/
         if(consola->posx >= NRO_COLUMNAS_VIDEO) {
            if(consola->posy == NRO_FILAS_VIDEO-1){
               subir(consola);
            }else{
               consola->posy++;
            }
         
           consola->posx = 0;
           flush(consola);
         }
         
         if(consola->cnt_ram == TAM_BUFFER_VIDEO) flush(consola);
         consola->mem_ram[consola->cnt_ram++] = consola->atrib | c;
         consola->posx++;      /* avanza el cursor a la prox. columna */
         return;
   }
}


/*===========================================================================*
 *===========================================================================*/
PRIVADA void subir(cons * consola)
{
   char *origen, *destino, *ult_linea;
   int i;
   
   /* saca lo que exista en el buffer de salida */
   flush(consola);
   
   /* la cantidad de bytes a mover es igual al total de bytes de la 
    * memoria de video para una pantalla menos la cant. de bytes de
    * la primera fila. esto es 7840 bytes
    */
   int cnt_bytes = (BYTES_VIDEO - BYTES_FILA);
   
   /* si el origen mas la cant. de memoria para una pantalla supera
    * el tope inferior de memoria asignada a esta pantalla entonces
    * tenemos que copiar todo el contenido actual de la consola al 
    * principio de la memoria asignada para esta.
    */
   if(consola->origen + DIM_VIDEO  >= consola->tope_inf){
      /* IMPORTANTE: (2 * xxx) es porque origen y tope_sup estan 
       * expresados en palabras y con respecto a la direccion de
       * memoria de video.
       */
      origen  = video + 2 * consola->origen;
      destino = video + 2 * consola->tope_sup;
      cp_mem((dir32)destino, (dir32)origen, cnt_bytes );
      consola->origen = consola->tope_sup;
   }else {
      consola->origen += NRO_COLUMNAS_VIDEO;
   }
   
   if (consola == cons_actual) CRTC(ORIGEN, consola->origen);
   
   flush(consola);
}


/*===========================================================================*
 *===========================================================================*/
PRIVADA void flush(cons * consola)
{
   unt16 pos_cur;
   
   /* Si hay caracteres en el buffer, los mandamos a la pantalla. */
   if (consola->cnt_ram > 0){
      cp_video(consola);
      consola->cnt_ram = 0;
   }
   
   /* chequeamos y actualizamos la posicion del cursor. */
   if (consola->posx < 0)                  consola->posx = 0;
   if (consola->posx > NRO_COLUMNAS_VIDEO) consola->posx = NRO_COLUMNAS_VIDEO;
   if (consola->posy < 0)                  consola->posy = 0;
   if (consola->posy >= NRO_FILAS_VIDEO)   consola->posy = NRO_FILAS_VIDEO -1;
   
   /* calculamos la posicion de memoria en palabras en la que 
    * debe encontrarse el cursor
    */
   pos_cur = consola->origen + (consola->posy * NRO_COLUMNAS_VIDEO) + consola->posx;
   if (pos_cur != consola->pos_cur) {
      if (consola == cons_actual) CRTC(CURSOR, pos_cur);
      consola->pos_cur = pos_cur;
   }
}


/*===========================================================================*
 *===========================================================================*/
PRIVADA void cp_video(cons *consola)
{
   unt16 * ptr;
   unt16 * dst;
   int  cnt, res;
   ptr = &consola->mem_ram[0];	/* ptr. al buffer de salida de la consola */
   cnt = consola->cnt_ram;          /* cnt. de bytes en el buffer de salida   */
   
   /* calculamos la posicion de memoria real en donde debemos
    * escribir los datos en la pantalla.
    */
   dst = (unt16 *)((unt32)video + (unt32)(2*consola->pos_cur));
   
   /* mientras existan caracteres por copiar */
   while(cnt--){
      
      /* si el caracter el un 0x00 ponemos en blanco todo hasta
       * el final de la fila.
       */
      if(*ptr & 0x00ff == 0x0000){
         res = NRO_COLUMNAS_VIDEO - (((char *)dst - video)/2)/NRO_COLUMNAS_VIDEO;
         while(res--)
            *dst++ = consola->atrib | ' ';
      }
      else *dst++ = *ptr;
   
      ptr++;
   }
}


/*===========================================================================*
 *                              CRTC                                         *
 *===========================================================================*/
PRIVADA void CRTC(int reg, unt16 val)
{
//  desactivar();
   outp8(P_6845_IND, reg +1);
   outp8(P_6845_DAT, BYTE(val));
   outp8(P_6845_IND, reg);
   outp8(P_6845_DAT, val>>8);
//   activar();
}



/*===========================================================================*
 *===========================================================================*/
extern terminal terminales[];

PUBLICA void inic_consola(terminal * t)
{
   cons * consola;
   int  c;
   
   c = t - &terminales[0];         /* c es el nro. de terminal */
   if(c >= NRO_CONSOLAS) return;
   
   consola = &consolas[c];
   consola->term = t;
   t->disp = consola;
   
   inic_segdatos(SEL_VIDEO,  /*(dir32)*/video, TAM_VIDEO, PRIV_KERNEL );
   
   /* asignamos la memoria que le corresponde a esta consola */
   /* a cada consola le corresponden dos pantallas completas */
   consola->tope_sup = 4 * c * DIM_VIDEO;
   consola->tope_inf = consola->tope_sup + 4 * DIM_VIDEO;
   consola->origen   = consola->tope_sup;
   consola->atrib    = 0x0700;
   consola->posx     = 0;
   consola->posy     = 1;
   consola->pos_cur  = 0;
   
   /* Limpiamos la pantalla. */
   cambiar_consola(0);
}


/*===========================================================================*
 *===========================================================================*/
PUBLICA void cambiar_consola(int c)
{
   if(c < 0 || c >= NRO_CONSOLAS) return;
   cons_actual = &consolas[c];
   CRTC(ORIGEN, cons_actual->origen);
   CRTC(CURSOR, cons_actual->pos_cur);
}


char tmp[200];

/* ====================================================================== *
 ==  Esta es mi version de printf()                                      ==
 =========================================================================*/
PUBLICA void mostrar(char *s, ...){
   cons * cons_sistema; 
   char *p= tmp;
   formatear(tmp, s, i_arg(s));       /* formatea la cadena */  
   cons_sistema = &consolas[0];
   
   /* mientras no se encuentre un caracter nulo lo enviamos a
    * la consola del sistema es decir la consola 0.
    */
   while(*p)
      out_char(cons_sistema, *p++);
   
   /* y por ultimo mandamos todo a la pantalla */
   flush(cons_sistema);
}

