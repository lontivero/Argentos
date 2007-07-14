/* ====================================================================== *
 ==  Este archivo contiene algunas funciones utiles para depurar el	 ==
 ==  sistema.										 ==
 ==  Estas son:										 ==
 ==    + ver_mem		.muestra el contenido de una dir. de memoria	 ==
 ==    + dump_descr	.muestra (y analiza) un descriptor			 ==
 ==    + ver_regs		.muestra todos los los registros de 386		 ==
 ==    + div0		(exc)	.provoca una excepcion  0 (div0)		 ==
 ==	 + falla_pagina	(exc) .provoca una excepcion 14 (page_fault)	 ==
 ==	 + int3		(exc) .provoca una excepcion  3 (break_point)	 ==
 ==	 + selector_invalido (exc) .provoca una excepcion  x 			 ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */

#include <macros.h>
#include <debug.h>
#include <multiarg.h>
#include <mem.h>
#include <sform.h>
#include "..\nucleo\mprot.h"
#include "..\nucleo\proc.h"
//#include "..\nucleo\disp\teclado.h"

//PRIVADA void dump_descr(selector sel);
PRIVADA void obt_cad();

int pow(int b, int e);
int obt_hex();
unt16 esp();

char buffentr[100];

void obt_cad()
{
   unt16 t;
   int i = 0;

   t=esp();
   mostrar("%c", t); 
   while(t!=27){
      buffentr[i++] = t;
      t = esp();
      mostrar("%c", t); 
   }
   buffentr[i++] = 0;
}

int pow(int b, int e)
{
   int i;
   int r=b;
   if(e==0) return 1;
   for(i=1; i<e; i++)
      r*=b;

   return r;
}

int obt_hex(){
  unt32 n=0;
  int i = 0;
  unt32 l;
  obt_cad();
  l = tam_cad(buffentr);

  for(i=0; i<l;i++)
     if(buffentr[i]>='0' && buffentr[i]<='9')
        n+=(buffentr[i]-'0')*pow(0x10,(l-i)-1);
     else
        n+=(buffentr[i]-'a'+10)*pow(0x10,(l-i)-1);
  
  mostrar("%d", n); 
  return n;
}

unt16 esp()
{
   unt16 t;

   while(TRUE)
   {
//      t = obt_car();
      if(t) return t;
   }
   return 0;
}



typedef struct{
  proc * cabeza;
  proc * cola;
} cola_proc;


extern proc tabla_proc[];
extern cola_proc colas[3];

/* ====================================================================== *
 == muestra el contenido de una porcion de memoria de parafos de 16 bytes== 
 ======================================================================== */
PUBLICA void ver_procs()
{
   proc * p;
   int i;
   char * estados[]={"listo     ", "libre     ", "enviando  ", "", "recibiendo"};

   mostrar("\n%9s  %4s  %12s  %10s  %12s  %9s", "direccion", "nro.", "proceso", "estado", "env. a", "pila");
   mostrar("\n%9s  %4s  %12s  %10s  %12s  %9s", "---------", "----", "-------", "------", "------", "-------");
   for(i=0; i<NRO_PROCESOS_TOTAL; i++)
   {
       p = &tabla_proc[i];     
       mostrar("\n%08X    %02d  %12s  %10s  %12s  %08X", p, p->nro, p->nombre, estados[p->estado], (p->estado==2)? tabla_proc[p->enviando_a].nombre : (p->estado==4)? tabla_proc[p->recibiendo_de].nombre : " ", p->p_tss.esp );             
   }

   mostrar("\n");
   for(p = colas[0].cabeza; p != NULL; p=p->prox)
       mostrar("%s->", p->nombre);
       
   mostrar("NULL\n");
}


/* ====================================================================== *
 == muestra el contenido de una porcion de memoria de parafos de 16 bytes== 
 ======================================================================== */
PUBLICA void ver_mem(unt32 direccion, unt32 cantidad){
   int i, j;
   unt8 *p, *q;

   p = (unt8 *)direccion;

   for(i=0; i<cantidad; i++){
      q = p;
      mostrar("\n");
      mostrar("%08X| ", p);
      for(j=0; j<8; j++) mostrar("%02X ", *p++); 
      mostrar("- ");
      for(j=0; j<8; j++) mostrar("%02X ", *p++); 
      mostrar("| ");

      for(j=0; j<16; j++){        
          mostrar("%c", (*q>' ' && *q<128) ? *q: '.');
          q++;
      }
   }
}


		


/* ====================================================================== *
 ==  muestra todos los registros de 386 a excepcion de los DRx. Tambien	 == 
 ==  hace uso de dump_descr para los registros de segmento.			 ==
 ======================================================================== */
PUBLICA void ver_regs(regs *r)
{
   mostrar("\n<Registros>");
   mostrar("\n\tEAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X  EFL=%08X", r->eax, r->ebx, r->ecx, r->edx, r->eflags);
   mostrar("\n\tESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X  EIP=%08X", r->esi, r->edi, r->ebp, r->esp, r->eip);

   mostrar("\n<Selectores>");
   mostrar("\nNOTA: (G)ranular - (C/D)Codigo/Datos - (P/A)Presente/Ausente - (012)Privilegios" );
   mostrar("\n      (R/W/X)Leeible/Escribible/Ejecutable.");
   mostrar("\n\tSelector (ind |ti|rpl)  base     limite   tipo ");
   mostrar("\n\t CS:%04X (%04X| %d| %d )  ", r->cs, r->cs>>3, (r->cs>>2)&1, r->cs&3);   dump_descr(r->cs);
   mostrar("\n\t DS:%04X (%04X| %d| %d )  ", r->ds, r->ds>>3, (r->ds>>2)&1, r->ds&3);   dump_descr(r->ds);
   mostrar("\n\t ES:%04X (%04X| %d| %d )  ", r->es, r->es>>3, (r->es>>2)&1, r->es&3);   dump_descr(r->es);
   mostrar("\n\t FS:%04X (%04X| %d| %d )  ", r->fs, r->fs>>3, (r->fs>>2)&1, r->fs&3);   dump_descr(r->fs);
   mostrar("\n\t GS:%04X (%04X| %d| %d )  ", r->gs, r->gs>>3, (r->gs>>2)&1, r->gs&3);   dump_descr(r->gs);
   mostrar("\n\t SS:%04X (%04X| %d| %d )  ", r->ss, r->ss>>3, (r->ss>>2)&1, r->ss&3);   dump_descr(r->ss);
   mostrar("\n");
}


/*
 * Para provocar errores
 */

/* ====================================================================== *
 ==  realiza una division por 0 provocando una excepcion 0			 ==
 ======================================================================== */
PUBLICA void div0(){
  unt16 i;
  i = 0;
  i = 3/i;
}

/* ====================================================================== *
 ==  intenta acceder a una direccion muy grande generando un fallo de 	 ==
 ==  de pagina. excepcion 14.								 ==
 ======================================================================== */
PUBLICA void falla_pagina(){
   unt8 *p;
   p = (unt8 *)0xfffff007;
   *p = 1;
}

/* ====================================================================== *
 ==  carga fs con el selector 0x80 (sel 16) que no existe generando 	 ==
 ==  una excepcion x. 									 ==
 ======================================================================== */
PUBLICA void selector_invalido(){
  asm("mov $0x80, %ax; mov %ax, %fs;");
}

/* ====================================================================== *
 ==  genera una excepcion 3 (break-point)					  	 ==
 ==  Asi, es posible insertarla en cualquier parte del codigo para	 ==
 ==  ver el contenido de los registros, memoria, etc.				 ==
 ======================================================================== */
PUBLICA void int3(){
  asm("int $0x3;");
}


/* ====================================================================== *
 ==  muestra un mensaje y detiene todo					  	 ==
 ==  Es una especie de panic() demasiado simplificada				 ==
 ======================================================================== */
char tmp[200];

PUBLICA void fatal(char *s, ...)
{
  desactivar();
  formatear(tmp, s, i_arg(s)); 				/* formatea la cadena */

  mostrar("\n%s\n", tmp);
  for(;;);
}

/* ====================================================================== *
 ==  detiene todo.								 	==
 ======================================================================== */
PUBLICA void pausa()
{
  desactivar();
  for(;;);
}


