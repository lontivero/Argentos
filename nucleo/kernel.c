/* ====================================================================== *
 ==  Este archivo contiene la funcion main() en C. Esta es llamada por   ==
 ==  mp.asm.                                                             ==
 ==  Basicamente inicializa:                                             ==
 ==    + la memoria                                                      ==
 ==    + las excepciones e interrupciones                                ==
 ==    + el pic                                                          ==
 ==    + y la tabla de procesos.                                         ==
 ==                                                                      ==
 ==   y gira la rueda (segun Tanembaum)                                  ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#include "kernel.h"
#include "mprot.h"
#include "reloj.h"
#include "irq.h"
#include "excep.h"
#include "disp\teclado.h"
#include "disp\terminal.h"



/* ====================================================================== *
 == Esta es la fnc. main()                                               ==
 =========================================================================*/
int  main(){
   inic_mp();
   inic_terms();
   inic_pic();
   inic_irqs();
   inic_excs();
   inic_reloj();
   inic_teclado();
   inic_procs();
   
   girar();
   while(1);

   return OK;
}
