/* ===================================================================== *
 ==  Este archivo contiene las funciones nacesarias para manejar las    ==
 ==  las destintas tablas del 386 en modo protegido.                    ==
 ==  Estas son:                                                         ==
 ==    + const_descr .construye un descriptor local o global segun el   ==
 ==             selector                                                ==
 ==    + const_idt   .construye un descriptor para una irq. o exc.      ==
 ==    + es_local    .determina si un selector es local o global        ==
 ==    + obt_dir     .dado un selector retorna un ptr. al descriptor    ==
 ==             discriminando si es local o global.                     ==
 ==    + obt_base    .dado un selector retorna la direccion base del    ==
 ==             descriptor correspondiente                              ==
 ==    + obt_limite  .dado un selector retorna el limite del            ==
 ==             descriptor correspondiente                              ==
 ==    + obt_tipo    .dado un selector retorna el tipo del descriptor   ==
 ==             correspondiente en nuestro formato (ver abajo)          ==
 ==    + inic_mp     .construye las tablas del sistema                  ==
 =========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++      ==
 ==    ++ Por: Lucas Ontivero                                           ==
 ==    ++ email: lucasontivero@yahoo.com.ar                             ==
 ====================================================================== */

#include <macros.h>
#include <mem.h>
#include "mprot.h"
#include ".\disp\consola.h"


/* se declaran aqui pero la mem. se asigna en memoria.c */
PRIVADA descriptor gdt[NRO_DESCRIPTORES_GDT];   
PRIVADA gate       idt[NRO_DESCRIPTORES_IDT];
PUBLICA tss        nucleotss;


PRIVADA bool  es_local(selector sel);
PRIVADA dir32 obt_base(descriptor * descr);
PRIVADA unt32 obt_limite(descriptor * descr);
PRIVADA unt16 obt_tipo(descriptor * descr);
PUBLICA void  inic_segcodigo(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios);
PUBLICA void  inic_segdatos(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios);


EXTERNA proc tabla_proc[];

/*=========================================================================*
 *                              init_codeseg                               *
 *=========================================================================*/
PUBLICA void inic_segcodigo(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios)
{
   const_descr(descr, base, limite, privilegios | SEG_CODIGO);
}


   
/*=========================================================================*
 *                              init_dataseg                              ==
 *=========================================================================*/
PUBLICA void inic_segdatos(descriptor * descr, dir32 base, unt32 limite, unt8 privilegios)
{
   const_descr(descr, base, limite, privilegios | SEG_DATOS);
}

/* ====================================================================== *
 == construye un descriptor local o global segun el selector             ==
 ======================================================================== */
PUBLICA bool const_descr(descriptor * descr, dir32 base, unt32 limite, unt16 tipo)
{
  /*
   * Construye un descriptor de segmento en la LDT o GDT y
   * la inicializa con los parametros indicados.
   */
   if(tipo & GRANULAR)         limite /= TAM_PAGINA;      
   else if(limite >=_1MB_)  return FALSE;  /* es imposible un limite > 1MB  
                                              si el seg. no es granular */  
   descr->limite00_15 = (unt16)limite;
   descr->limite16_23 = (unt8)(((tipo & 0xf000)>> 8) | ((limite & 0x000f0000) >> 16) | DEFAULT );
   descr->base00_15   = (unt16)base;
   descr->base16_23   = (unt8)(base >> 16);
   descr->base24_31   = (unt8)(base >> 24);
   descr->tipo        = (unt8)(tipo & 0x00ff);
   
   return TRUE;
}


/* ====================================================================== *
 == construye un descriptor para una irq. o exc.                         ==
 ======================================================================== */
PUBLICA void const_idt(unt16 nro, dir32 direccion, unt8 tipo)
{
   gate *d = &idt[nro];
   
   d->sel  = SEL_CODIGO_LINEAL;
   d->tipo = tipo; 
   d->direccion00_15  = (unt16)direccion;
   d->direccion16_31  = direccion >> 16;
}

/* ====================================================================== *
 == determina si un selector es local o global                           ==
 ======================================================================== */
PRIVADA bool es_local(selector sel)
{  return (sel & ES_LOCAL ? TRUE: FALSE); }



/* ====================================================================== *
 == dado un selector retorna la direccion base del descriptor correspond ==
 ======================================================================== */
PRIVADA dir32 obt_base(descriptor * descr)
{
   return descr->base00_15 + (descr->base16_23<<16) + (descr->base24_31<<24);
}

/* ====================================================================== *
 == dado un selector retorna el limite del descriptor correspondiente    ==
 ======================================================================== */
PRIVADA unt32 obt_limite(descriptor * descr)
{
   unt32       limite;
   
   limite = descr->limite00_15 + ((descr->limite16_23 & 0x0f) << 16);
    
   if(descr->limite16_23 & GRANULAR){
      limite *= TAM_PAGINA;
      limite += 0x0fff;
   }

   return limite;
}


/* ====================================================================== *
 == dado un selector retorna el tipo del descriptor correspondiente      ==
 == Manejamos los tipos como un (unt16) con el siguiente formato.        ==
 ==                                                                      ==
 == 15  14  13   12   11   10    9    8    7    6   5    4   3  2  1  0  ==
 ==  0   0   0    0  [ G ][ D ][ 0 ][AVL][ P ][  DPL  ][DT ][   TIPO    ]==
 ==                                                                      ==
 == Las constantes para estos se encuentran en mprot.h                   ==
 ======================================================================== */
PRIVADA unt16 obt_tipo(descriptor * descr)
{
   return ((descr->limite16_23 << 12) | descr->tipo);
}


/* ====================================================================== *
 == buscar un descriptor en la GDT con tipo = 0 y si lo encuentra,       ==
 == devuelve el selector, de lo contrario retorna el selector NULO      == 
 ======================================================================== */
PRIVADA descriptor * obt_nuevo_descr()
{
   unt16 entrada;

   for(entrada = 0; entrada<NRO_DESCRIPTORES_GDT; entrada++)
      if(gdt[entrada].tipo == 0)
         return &gdt[entrada];
      
   return NULL;
}



/* ====================================================================== *
 ==  muestra y analiza un descriptor                                     ==
 ======================================================================== */
PUBLICA void dump_descr(selector sel)
{ 
   char  buf[2]={0,0};
   unt8  tipo;
   unt16 granularidad;
   
   descriptor * descr;
   descr = &gdt[sel / TAM_DESCR];
   granularidad = obt_tipo(descr);
   tipo  = (unt8)granularidad;

   mostrar("%08X %08X %04X", obt_base(descr), obt_limite(descr), tipo);
   
   if(granularidad & GRANULAR) mostrar(" G");
   else                        mostrar(" -");
                                            
   if(tipo & 8 /*SEG_CODIGO*/) mostrar("C");
   else                        mostrar("D");
                                            
/*   if(tipo & SEG_OP_32B)     mostrar("3");
   else                        mostrar("1");
*/                                          
                                            
   if(tipo & PRESENTE)         mostrar("P");
   else                        mostrar("A");
   
   /* nivel de privilegio DPL */
   buf[0] = ((tipo & DPL_MASCARA) >> 5) + '0';
   mostrar(buf);
   
   if(tipo & 0x010)           mostrar("A");
   else                       mostrar("S");
   
   
   if(tipo & 8 /* SEG_CODIGO */ ){
      if(tipo & LEIBLE)       mostrar("R");
      else                    mostrar("X");
   }else{
      if(tipo & ESCRIBIBLE)   mostrar("W");
   }

   
   if(tipo & ACCEDIDO)        mostrar("A");
   else                       mostrar("-");  
}


/* ====================================================================== *
 == construye y carla las tablas del sistema (GDT, LDT, IDT y TSS del    ==
 == nucleo) y cambia de tarea.                                           ==
 ======================================================================== */
PUBLICA void inic_mp()
{
   pseudo td;
   extern unt32 start;
   unt8 nro_proc;
   unt8 indice;
   struct proc * p_proc;
 
   /*
   *  Limpio las tablas GDT, LDT e IDT para posteriormente asignarles
   *  los valores correspondientes.
   */
   pt_mem((dir32)gdt, 0, NRO_DESCRIPTORES_GDT * sizeof (descriptor));
   pt_mem((dir32)idt, 0, NRO_DESCRIPTORES_IDT * sizeof (gate));
   pt_mem((dir32)&nucleotss, 0, sizeof(tss));

   /* Construyo los descriptores.  */
    
   /*  El NULO  */
   const_descr(SEL_NULO   ,   0, 0, 0);
 
   /*  FLAT codigo y datos   */
   inic_segcodigo(&gdt[IDX_CODIGO_LINEAL], 0, 0xFFFFFFFF, PRIV_KERNEL);
   inic_segdatos (&gdt[IDX_DATO_LINEAL],   0, 0xFFFFFFFF, PRIV_KERNEL);
    
 
   /*  GDT, LDT e IDT  */
   inic_segdatos(&gdt[IDX_GDT], (dir32)gdt, NRO_DESCRIPTORES_GDT * sizeof (descriptor), PRIV_KERNEL);
   inic_segdatos(&gdt[IDX_IDT], (dir32)idt, NRO_DESCRIPTORES_IDT * sizeof (descriptor), PRIV_KERNEL);
 
 
   /* Para cada entrada en la tabla de procesos, construye descriptores
    * locales en GDT para las LDT en la tabla de procesos. 
    */
   for(nro_proc = 0; nro_proc < NRO_PROCS; nro_proc++){
      p_proc = &tabla_proc[nro_proc];
      indice = PRIMER_IDX_LIBRE + 2 * nro_proc;
 
      inic_segdatos(&gdt[indice], (dir32)p_proc->ldt, sizeof(p_proc->ldt), PRIV_KERNEL);
      gdt[indice].tipo = SEG_LDT;
      p_proc->selector_ldt = indice * TAM_DESCR;
 
      inic_segdatos(&gdt[indice+1], (dir32)&p_proc->p_tss, sizeof(p_proc->p_tss), PRIV_KERNEL);
      gdt[indice].tipo = SEG_TSS;
      p_proc->selector_tss = (indice+1) * TAM_DESCR;
      p_proc->p_tss.ldt = p_proc->selector_ldt;
   }
   
 
   //   _CR3(direccion);
   //   _CR0( CR0() | CR0_TS );
   //   goto saltito; saltito:
 
   /* Ya tengo paginacion. Que sensacion extraña!! :) */
 
   /*
    *  Lo ultimo.
    *  cargo las tablas GDT, LDT e IDT
    */
   
   /*  Cardo la GDT  */
   td.direccion = (dir32)gdt;
   td.limite    = NRO_DESCRIPTORES_GDT * sizeof (descriptor);
   LGDT(td);
 
   /*  Cardo la IDT  */
   td.direccion = (dir32)idt;
   td.limite    = NRO_DESCRIPTORES_IDT * sizeof (gate);
   LIDT(td);
   
   
   /* Construye el TSS principal.
    * Este se usa solamente para recordar el valor de los registros de la pila 
    * para poder recuperarlos luego de una interrupcion.
    * El puntero se establece cuando una interrupcion ocurre y el hardware
    * lo salva automaticamente. 
    */
   inic_segdatos(&gdt[IDX_NUCLEO_TSS], (dir32)&nucleotss, sizeof(nucleotss), PRIV_KERNEL);
   gdt[IDX_NUCLEO_TSS].tipo = SEG_TSS | PRIV_KERNEL;
   nucleotss.esp0   = nucleotss.esp    = MEM_CODIGO-2;
   nucleotss.ss0    = SEL_DATO_LINEAL;
   nucleotss.eflags = 0x202;
   nucleotss.eip    = start;
   nucleotss.cs     = SEL_CODIGO_LINEAL;
   nucleotss.ds     = nucleotss.es = nucleotss.ss = nucleotss.fs = nucleotss.gs = SEL_DATO_LINEAL;
/*   nucleotss.iomap= sizeof(nucleotss); */


    while(1);   
   ltr(SEL_NUCLEO_TSS);


}


/*************************************************************
 * resetear
 ************************************************************/
PUBLICA void resetear()
{
   unt32 idt_no_valido[2] = {0,0};
   pseudo td;
   
   td.direccion = (dir32)idt_no_valido;
   td.limite    = sizeof (gate);
   LIDT(td);
   int3();
}


/*===========================================================================
 ==                                                                        ==
 ==========================================================================*/
PUBLICA void asignar_segmentos(proc * p_proc)
{
/* This is called only by do_newmap, but is broken out as a separate function
 * because so much is hardware-dependent.
 */

   unt8  privilegios;
   tss * p_tss;
   
   privilegios = es_tarea(p_proc->nro) ? PRIV_KERNEL : PRIV_USUARIO;
   inic_segcodigo(&p_proc->ldt[IDX_CS_LDT],
                  p_proc->mapa[T].base, p_proc->mapa[T].tam,
                  privilegios);
   
   inic_segdatos(&p_proc->ldt[IDX_DS_LDT],
                 p_proc->mapa[D].base, p_proc->mapa[D].tam,
                 privilegios);
   
   inic_segdatos(&p_proc->ldt[IDX_SS_LDT],
                 p_proc->mapa[P].base, p_proc->mapa[P].tam,
                 privilegios);
   
   p_tss = &p_proc->p_tss;
   p_tss->cs = (IDX_CS_LDT * TAM_DESCR) | ES_LOCAL | privilegios;
   p_tss->ds = 
   p_tss->es = 
   p_tss->fs = 
   p_tss->gs = (IDX_DS_LDT * TAM_DESCR) | ES_LOCAL | privilegios;
   p_tss->ss = (IDX_SS_LDT * TAM_DESCR) | ES_LOCAL | privilegios;
}
