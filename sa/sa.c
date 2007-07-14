/* ====================================================================== *
 ==  Este archivo contiene las funciones nacesarias para inicializar	 ==
 ==  la memoria del kernel, sus tablas, descriptores, mapa de paginas.   ==
 ==  Estas son:										 ==
 ==    + est_man_irq	.establece una funcion como manejador de la irq	 ==
 ==				 especificada y la activa.			 	 ==
 ==    + inic_irq		.llena tabla de irqs con una funcion tonta.	 ==
 ==    + aviso_irq	.la funcion antes mencionada				 ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */

#include <debug.h>
#include <mem.h>
#include <macros.h>
#include <mensajes.h>

#include "..\nucleo\mprot.h"
#include "..\nucleo\memoria.h" 
#include "..\nucleo\proc.h"
#include "cache.h"
#include "sa.h"

PRIVADA void inic_sa();

/*
*/
PUBLICA void t_sa()
{
   mensaje m;
   inic_sa();

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
PRIVADA void inic_sa()
{
  mostrar("\nInicializando Servidor de Archivos...");
  inic_cache();

#if 0
  load_super(ROOT_DEV);         /* load super block for root device */

  /* Initialize the 'fproc' fields for process 0 .. INIT. */
  for (i = 0; i <= LOW_USER; i+ 1) {
        if (i == FS_PROC_NR) continue;  /* do not initialize FS */
        fp = &fproc[i];
        rip = get_inode(ROOT_DEV, ROOT_INODE);
        fp->fp_rootdir = rip;
        dup_inode(rip);
        fp->fp_workdir = rip;
        fp->fp_umask = ~0;
  }
#endif

  
}

