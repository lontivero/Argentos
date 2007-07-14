#include <mem.h>

void cp_mem(dir32 destino, dir32 origen, unt32 cantidad )
{
      int8 *po=(int8 *)origen;
	int8 *pd=(int8 *)destino;

	if(po == pd) return;
	if(pd < po || pd >= po + cantidad)
	{
		while(cantidad--)
      	     	*pd++ = *po++;
      }else {
		pd += cantidad - 1;
            po += cantidad - 1;

            while (cantidad--)
            	*pd-- = *po--;
      }	
}

void pt_mem(dir32 destino, int8 c, unt32 cantidad)
{
	int8 *p = (int8 *)destino;
      while (cantidad--)
           	*p++ = c;
}


unt32 tam_cad(char * cad)
{
      unt32 tam = 0;
      while(*cad++) tam++;
      return tam;
}

void cp_cad(char * destino, char * origen)
{
     cp_mem((dir32)destino, (dir32)origen, tam_cad(origen)+1);
}

