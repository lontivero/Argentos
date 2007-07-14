/* ====================================================================== *
 ==  Este archivo contiene las funciones nacesarias para inicializar     ==
 ==  la memoria del kernel, sus tablas, descriptores, mapa de paginas.   ==
 ==  Estas son:                                                          ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <mem.h>
#include <macros.h>
#include <mensajes.h>
#include "mprot.h"
#include "memoria.h"
#include "proc.h"
#include "otros.h"
#include "disp\consola.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define ALINEAR(dir)          dir = (dir + 0x00000FFF) & 0xFFFFF000
#define ENTRADAS_PAGINA       1024


unt32 inic_mem_disp;
unt32 cant_mem_tot;
unt32 * mapa_mem;
unt32 * dir_tablas;
unt32 * tablas_pags;
unt32 cant_pags_usadas;
unt32	marcos_pags;

PRIVADA void      inic_mem_fisica();
PRIVADA cod_err   est_pag(unt32 pagina);
PRIVADA cod_err   lib_pag(unt32 pagina);
PRIVADA unt32     obt_mem();
PRIVADA unt32     obt_pagl();




/*
*/
PUBLICA void t_mem_fisica()
{
	mensaje m;
	inic_mem_fisica();
	
	while(TRUE)
	{
		recibir(CUALQUIERA, &m);
		/*
		switch()
		{
			case :  break;
			case :  break;
			case :  break;
			case :  break;
			default: break;
		}
		*/
	
	}
}




/*
*/
PRIVADA dir32 prox_dir(int cant)
{
   dir32 ret = inic_mem_disp;
   
   inic_mem_disp += cant * _4KB_;
   ALINEAR(inic_mem_disp);

   return ret;
}
 

/*
*/
PRIVADA void inic_mem_fisica()
{
   extern int   end;
   extern int   start;

   unt32 pags_mapa_mem;
   unt32 direccion;
   int32 tabla, entrada;
   unt32 cant_tablas_pags;

   mostrar("\nIniciando Administrador de Memoria");

   inic_mem_disp  = (dir32)&end;
   cant_mem_tot   = obt_memoria();
   mostrar("\n\tMemoria total: %d", cant_mem_tot);

   /*
   *  Calcula la cantidad de paginas necesarias para contener el mapa
   *  de bits de las paginas d memoria.
   */
   cant_pags_usadas = 0;

   /* Nro. de marcos de paginas */
   marcos_pags    =   cant_mem_tot / TAM_PAGINA;
   marcos_pags   +=  (cant_mem_tot % TAM_PAGINA) ? 1 : 0;

   pags_mapa_mem  =  marcos_pags / TAM_PAGINA;
   pags_mapa_mem += (marcos_pags & TAM_PAGINA) ? 1 : 0;
  
   mapa_mem = (unt32 *)prox_dir(pags_mapa_mem);

   /*
   *  Reservo memoria para el directorio de tablas (4kb)
   */
   dir_tablas = (unt32 *)prox_dir(1);
   mostrar("\n\tDirectorio de Paginas: %0x08X", dir_tablas);


   /*
   *  Calcula la cantidad de tablas de paginas necesarias para manejar
   *  la memoria.
   *  Cada tabla puede manejar 4 MBytes. 
   */
   cant_tablas_pags  =  cant_mem_tot / _4MB_;   
   cant_tablas_pags += (cant_mem_tot % _4MB_) ? 1 : 0;

   tablas_pags = (unt32 *) prox_dir(cant_tablas_pags);
   mostrar("\n\ttabla paginas: %0x08X", tablas_pags);


   /******
   *  Ahora debo llenar el directorio de tablas con las
   *  direcciones de las tablas (no me digas)
   *  Voy a hacer un mapeo 1 a 1.
   */
   for (tabla = 0; tabla < cant_tablas_pags; tabla++)
      dir_tablas[tabla]  = ((dir32)tablas_pags + (tabla * TAM_PAGINA)) | 3;

   for (; tabla < 1024; tabla++)
      dir_tablas[tabla] = 0;

   /******
   *  Ahora debo asignar a cada entrada en cada tabla 
   *  de paginas, la direccion de memoria correspondiente
   */
   direccion = 0;
   for (tabla = 0; tabla < cant_tablas_pags; tabla++)
   {
      for (entrada = 0; entrada < ENTRADAS_PAGINA; entrada++)
      {
      if (direccion < cant_mem_tot)
         tablas_pags[tabla * 1024 + entrada] = direccion | 3;
      else
         tablas_pags[tabla * 1024 + entrada] = 0;

      direccion += TAM_PAGINA;   /* proxima pagina */
      }
   }

   mostrar("\nTablas de Paginas..Ok");

   /*****
   *  Ahora limpio el mapa de bit de paginas. Es decir, todas la paginas estaran
   *  libres y despues establezco que desde <start> hasta <inic_mem_disp>
   *  estan ocupadas. Tambien la memoria de video.
   */
 //  pt_mem((dir32)mapa_mem, 0, pags_mapa_mem*TAM_PAGINA);
   
   for(direccion = (dir32)&start; direccion < inic_mem_disp; direccion += TAM_PAGINA)
      est_pag(direccion/TAM_PAGINA);

   est_pag(MEM_VIDEO/TAM_PAGINA);   
}


#define E_PAG_USADA  -7


/*
*  marca una pagina (en el mapa de bits) como asignada
*  si ya estaba asignada emite un error.
*/
PRIVADA
cod_err est_pag(unt32 pagina)
{
   int i, bit;
   unt32 mascara;

   if(pagina * TAM_PAGINA > cant_mem_tot)
   {
      mostrar("pagina %d esta fuera de memoria", pagina);
      return E_PAG_USADA;
   }
   
   i   = pagina/32;
   bit = 31 - pagina%32;
   mascara = 1<<bit;

   mapa_mem[i] |= mascara;
   cant_pags_usadas++;

   return OK;
}


#define E_PAG_FUERA  -6

/*
*  marca una pagina (en el mapa de bits) como no asignada
*  si ya estaba asignada emite un error.
*/
PRIVADA
cod_err lib_pag(unt32 pagina)
{
   int i, bit;
   unt32 mascara;

   if(pagina * TAM_PAGINA > cant_mem_tot)
   {
      mostrar("pagina %d esta fuera de memoria", pagina);
      return E_PAG_FUERA;
   }
     
   i   = pagina/32;
   bit = 31 - pagina%32;
   mascara = 1<<bit;

   mapa_mem[i] &= ~mascara;
   cant_pags_usadas--;
   return OK;
}

#define E_SIN_MEM		-8

PRIVADA
unt32 obt_pagl()
{
   int i, j;
   unt32 pag;

   for(i=0; i< marcos_pags; i++)
      if(mapa_mem[i] <0xffffffff)
         for(j=0; j<32; j++)
            if(!(mapa_mem[i] & (0x80000000 >> j)))
            {
               pag = (i*32) + j;
               est_pag(pag);
               return pag;
            }

   return E_SIN_MEM;
}


