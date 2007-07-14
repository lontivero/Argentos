
#include <const.h>

unt8  * monton = (unt8 *)MEM_MONTON;

unt8  * alojar(unt32 bytes)
{
   unt8 * ret;

   /* Si sobrepasa el limite retorno NULL  */
   if((monton + bytes + 4) > FIN_MONTON){
      fatal("No hay mas memoria disponible en el monton.");
   return NULL;
   }

   *((unt32 *)monton) = bytes;
   ret = monton + 4;
   monton += bytes;
   pt_mem(ret, 0, bytes);
   return ret;
}

void liberar(void * ptr)
{
   dir32 dptr = (dir32)ptr;
   if(dptr < (MEM_MONTON + 4) || dptr >= FIN_MONTON)
      fatal("Intenta liberar memoria que no pertenece al monton.");
   
   
   unt32 bytes = *((unt32 *)(ptr - 4));
   if(bytes <= 0)
      fatal("Algo anda mal en liberar.");
   
   
}

ulong
get_heap_ptr()
{
   return((ulong)pheap_lo);
}

ulong
get_heap_end()
{
   return((ulong)pheap_hi);
}
