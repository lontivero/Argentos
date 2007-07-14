#include <debug.h>
#include <mensajes.h>
#include "cache.h"
#include "..\nucleo\proc.h"

#define NRO_BUFFERS	100
#define MASCARA_HASH	(NRO_BUFFERS-1)

#define LEER		1
#define ESCRIBIR		2

#define LIMPIO		1
#define SUCIO		2

PRIVADA void operar(buffer * b, int op);
PRIVADA void sacar(buffer * b);
PRIVADA void bajar_todo(int disp);

PRIVADA buffer buffers[NRO_BUFFERS];
PRIVADA buffer * buf_cabeza;
PRIVADA buffer * buf_cola;
PRIVADA buffer * hash[NRO_BUFFERS];

PUBLICA buffer * obt_buffer(int disp, int nro_bloque, bool es_vital)
{
   struct buffer * p_buffer;
   struct buffer * p_anterior;
   int i;

   p_buffer = hash[nro_bloque & MASCARA_HASH];

   /*
    * Buscamos el bloque del dispositivo indicado. y si
    * lo encontramos lo retornamos.
    */
   for(; p_buffer != NULL; p_buffer = p_buffer->hash){
       if(p_buffer->bloque == nro_bloque && p_buffer->disp == disp)
       {
          /* si no tiene uso, la quitamos de la lista */
          if(p_buffer->usos == 0) sacar(p_buffer); 
          p_buffer->usos++;
          return p_buffer;
       }
   }
   
   /*
    * Si estamos aca es porque no lo encontramos. Asi que debemos asignarle
    * un nuevo bloque cache. Si la lista de bloques libres esta vacia panico.
    */
   if(buf_cabeza == NULL) fatal("No hay buffers libres :(");
   p_buffer = buf_cabeza;
   sacar(p_buffer);

   /* Eliminamos el bloque de la lista de bloques libres. */
   i = hash[p_buffer->bloque];
   p_anterior = hash[i];
   if(p_anterior == p_buffer) hash[i] = p_buffer->hash;
   else
      for(; p_anterior->hash != NULL; p_anterior = p_anterior->hash)
         if(p_anterior->hash == p_buffer){
              p_anterior->hash = p_buffer->hash;
              break;
         }
   
   /*
    * Si ha sido modificado hay que grabarlo; y ya que vamos a 
    * mover el disco aprovechamos para grabar todos los bloques 
    * que le pertenecen a este dispositivo.
    */
   if(p_buffer->estado == SUCIO) bajar_todo(p_buffer->disp);

   p_buffer->usos   = 0;
   p_buffer->bloque = nro_bloque;
   p_buffer->disp   = disp; 
   p_buffer->vital  = es_vital;
   p_buffer->hash   = hash[nro_bloque & MASCARA_HASH];
   hash[nro_bloque & MASCARA_HASH] = p_buffer;

   operar(p_buffer, LEER);

   return p_buffer;
}



PUBLICA void est_bloque(struct buffer *p_buffer)
{
   if(p_buffer == NULL) return;

   p_buffer->usos--;

   /* si esta siendo usado, retornar */
   if (p_buffer->usos != 0) return; 
  
   /* ponemos el bloque al ultimo */
   p_buffer->ant = buf_cola;
   p_buffer->prox = NULL;

   if(buf_cabeza == NULL)
      buf_cabeza = p_buffer;
   else
      buf_cola->prox = p_buffer;

   buf_cola = p_buffer;

   /* si es un bloque importante y esta sucio lo escribimos y
    * al igual que antes, no movemos el disco para un solo blk.
    */
   if(p_buffer->vital && p_buffer->estado==SUCIO)
        bajar_todo(p_buffer->disp);
}



PRIVADA void operar(buffer * b, int op)
{
   mensaje m;
  
   m.POSICION  = b->bloque * TAM_BLOQUE;
   m.SERVICIO  = op == LEER ? LEER_DISP : ESCRIBIR_DISP;
   m.CANTIDAD  = TAM_BLOQUE; 
   m.DIRECCION = (dir32) b->d;

   envrec(b->disp, &m);

   b->estado = LIMPIO;
}
	
	
	
PRIVADA void sacar(struct buffer * p_buffer)
{
   if(p_buffer->ant == NULL )	buf_cabeza = p_buffer->prox;
   else      		p_buffer->ant->prox = p_buffer->prox;

   if(p_buffer->prox == NULL)	buf_cola   = p_buffer->ant;
   else			p_buffer->prox->ant = p_buffer->ant;
}


PRIVADA void bajar_todo(int disp)
{
   buffer * b;
   for(b = buf_cabeza; b <= buf_cola; b++)
       if(b->disp == disp)
       {
          operar(b, ESCRIBIR);
          b->estado = LIMPIO;
       }
}



PUBLICA void inic_cache()
{
   buffer * b;

   buf_cabeza = &buffers[0];
   buf_cola   = &buffers[NRO_BUFFERS - 1];

   for(b = buf_cabeza; b <= buf_cola; b++)
   {
      b->bloque = -1;
      b->disp   = -1;
      b->prox   = (b+1);
      b->ant    = (b-1);
   }

   buf_cabeza->ant = NULL;
   buf_cola->prox  = NULL;

   for(b = buf_cabeza; b <= buf_cola; b++) b->hash = b->prox;
   hash[0] = buf_cabeza;
}
	
	
