/* ====================================================================== *
 ==  Este archivo contiene las funciones nacesarias para manejar las     ==
 ==  irqs.                                                               ==
 ==  Estas son:                                                          ==
 ==    + est_man_irq .establece una funcion como manejador de la irq     ==
 ==             especificada y la activa.                                ==
 ==    + inic_irq    .llena tabla de irqs con una funcion tonta.         ==
 ==    + aviso_irq   .la funcion antes mencionada                        ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <macros.h>
#include "irq.h"
#include "disp\consola.h"

/*
   REGISTROS
   ---------

   IRR - Interrupt Request Register
   ISR - In Service Register
   IMR - Interrupt Mask Register

   Estos 3 registros son de 8 bits c/u los cuales corresponden a las 8 interrupciones de
   hardware.

   - El IRR se conecta directamente a las 8 lineas de interrupciones (IR0 a IR7). Al recibir
   una interrupcion, el origen de esta int, establece la linea que le corresponde a HIGH y 
   por lo tanto el correspondiente bit en IRR tambien es establecido. De esta forma, el PIC
   sabe cual es el origen de la int.

   - El IMR puede ser usado para enmascarar o ignorar uno o mas origenes de ints. Si un bit
   en el IMR es establecido, entonces una int. del correspondiente origen será ingnorada.

   - Para las ints. no enmascaradas, el PIC debe determinar que ints. debe presentar a la 
   CPU. Dado que pueden existir otras ints. esperando el servicio, el PIC seleccona el bit
   del IRR que tenga el valor mas bajo y por ende la mayor prioridad. Este es comparado con
   el contenido del ISR el cual indica el origen de la int. mas recientemente recibida. Si 
   el IRR tiene la prioridad mas alta esperando el servicio, entonces el PIC pasa esta al 
   CPU. El PIC continua con esta logica hasta que todas las ints. con alta prioridad hayan 
   sido servidas.

   Cuando el PIC pasa una int. al CPU, este establece el bit correspondiente en el registro
   ISR (en servicio). Al mismo tiempo el correspondiente bit es limpiado del registro IRR 
   (esperando). Asi, mantiene la pista de cual interrupcion esta siendo manejada y no 
   intenta servirla nuevamente.

   CASCADA
   -------
   El PIC esta limitado a solo 8 origenes por lo que ponerlos en casacada es la solucion 
   para enlazar varios PICs. La linea EN determina si el PIC opera como maestro o esclavo.
   Cuando un voltage es aplicada a la linea EN, el PIC actua como maestro. De lo contrario 
   funciona como esclavo.

   INICIALIZANDO EL PIC
   --------------------
   Como su nombre lo sugiere, el PIC es programable y responde a varios comandos.
   Basicamente se encuentran en 2 categorias de comandos:
     - Comandos de Inicializacion  (ICW)
     - Comandos de Operacion       (OCW)
   
   Los ICW (comandos de inicializacion) son enviados al PIC en un orden muy preciso puesto
   que estos son interdependientes. Los OCW pueden enviarse en cualquier orden.

   Dos puertos se usan para comunicarse con el PIC. Para el maestro se usan el 0x20 y 0x21
   miestras que para el esclavo son el 0xA0 y 0xA1. La inicializacion siempre comienza 
   enviando el primer comando de inic. ICW1 al puerto 0x20 o 0xA1 dependiendo de que PIC se 
   quiera inicializar.

   ICW1 esta compuesto de 8 bits. Solo algunos bits son significativos para este tema: Si el
   bit 0 esta en '0' significa que el PIC debe prepararse para recibir los siguientes 3 
   comandos de inicializacion ICW2, ICW3, ICW4. El bit 3 especifica como debe el PIC 
   responderle al dispositivo de origen. Si es 0, el PIC responde como un disparador de pulso 
   y en tal caso establece el IRQx a HIGH y luego lo vuelve a LOW. Mientras que si es 1, 
   responde estableciendo el IRQx a HIGH lo mantiene hasta que el origen lo pone en LOW.

     7   6   5   4   3   2   1   0
   +---+---+---+---+---+---+---+---+
   | x | x | x | x |   | 1 |   |   |
   +---+---+---+---+---+---+---+---+
                     |       |   |
                     |       |   +--> Debe esperar los comandos ICW2, ICW3, ICW4?
                     |       |        ( 0 = NO;   1 = SI )
                     |       +------> 1 = Solo un PIC disponible.
                     |                0 = PICs en cascada
                     +--------------> 0 = Pulsos
                                      1 = Constante

   El BIOS establece el ICW1 con el valor 00010001b. Esto es, se le enviaran 3 palablas de 
   control de inicializacion, se usan PICs en cascada y envia señales en forma de pulsos.

      mov   al, 00010001b
      out   020h, al       ; Envia ICW1 al PIC maestro
      out   0A0h, al       ; Envia ICW1 al PIC esclavo

   ICW2 siempre sigue a ICW1. ICW2 define la direccion base para la primera interrupcion 
   (IRQ0). La segunda int. (IRQ1) es disparada entonces en la dir. base +1; la tercera (IRQ2)
   en dir. base +2 y asi. El BIOS establece el ICW2 en 08h para el maestro y en 70h para el 
   esclavo, es decir que el IRQ0 (el clock) es establecido por el BIOS como la int. 8, el 
   IRQ1 (el teclado) es la int 9 y asi.
   
      mov   al,  08h       ; 08h es la interrupcion base para el maestro
      out   21h,  al       ; envia el ICW2 al maestro
      mov   al,  70h       ; 70h es la interrupcion base para el esclavo
      out   0a1h, al       ; envia ICW2 a el esclavo

   ICW3 sigue a ICW2 y debe ser enviada al PIC si el PIC esta en cascada. Si el bit 1 del 
   ICW1 es 0 (PIC en cascada), entonces el PIC espera el ICW3. El ICW3 tiene la tarea de 
   informar al maestro y al esclavo sobre el control de las interrupciones. (Como estan 
   conectadas)

     7   6   5   4   3   2   1   0
   +---+---+---+---+---+---+---+---+
   |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     |   |   |   |   |   |   |   |
     |   |   |   |   |   |   |   +---> 0 = IRQ0 conectado al hardware
     |   |   |   |   |   |   |         1 = IRQ0 conectado al esclavo (en cascada)
     |   |   |   |   |   |   +-------> 0 = IRQ1 conectado al hardware
     |   |   |   |   |   |             1 = IRQ1 conectado al esclavo (en cascada)
     +---+---+---+---+---+-----------> Lo mismo para todas la IRQxs.

   Para el maestro, cada bit en este registro corresponde a una de las lineas de interrupcion
   IRQ0 a IRQ7, y todas se conectan directamente al hardware. El BIOS establece solo el bit 2
   para indicar que el PIC esclavo esta en cascada en la linea IRQ2.

      mov   al, 00000100b  ; cascada sobre IR2
      out   21h, al        ; envia ICW3 al maestro

   Para el esclavo el ICW3 contiene el numero de control de interrupcion. El BIOS establece 
   el ICW3 para el esclavo con el valor 3.

      mov   al, 2          ; cascada sobre IR2
      out   0a1h, al       ; envia ICW3 al esclavo


   El ICW4 indica como se maneja el final de una int. Un PIC puede ser programado para que 
   automaticamente señale la finalizacion de una interrupcion o para que se señalen por 
   software. El una PC, el PIC requiere la asistencia del software para establecer el bit 1
   a 0. Esto tiene ciertas implicaciones para el manejador de interrupciones.

     7   6   5   4   3   2   1   0
   +---+---+---+---+---+---+---+---+
   | 0 | 0 | 0 | 0 | 0 | 0 |   |   |
   +---+---+---+---+---+---+---+---+
                             |   |
                             |   +---> 1 = El PIC trabaja con un procesador de la
                             |         serie Intel 80x86.
                             +-------> 0 = Terminacion manual de las ints.
                                       1 = Terminacion automatica de ints.

   CONTROL DEL PIC (OCW)
   ---------------------
   El PIC distingue entre varios OCWs por su disposicion y el puerto sobre el que se envian.

   - OCW1 permite cambiar el registro IMR (Interrupt Mask Register). Cambiando el contenido de
   este se puede suprimir o reactivar una interrupcion de hardware en particular. Si el bit
   es 1 la linea esta enmascarada y el dispositivo correspondiente no puede disparar ints.
   hasta que este bit sea puesto nuevamente en 0.
   OCW1 es enviado al segundo puerto del PIC (21h para el maestro y A1h para el esclavo).
   Normalmente todas las ints. estan habilitadas, es decir todo el IMR es 0. Si se pone a
   1 la entrada 2 (correspondiente al PIC esclavo), se deshabilitan todas las ints. que 
   maneja el esclavo.
   
   - OCW2 se usa principalmente para señalar la finalizacion de un manejador de ints. 
   Tambien se utiliza para la rotacion de prioridades y el manejo automatico de fin de 
   ints. Para PCs, ninguna de estas caracteristicas esta disponible. El PIC reconoce un
   OCW2 solo si este es enviado al primer puerto (20h para el maestro y A0 para el esclavo)

     7   6   5   4   3   2   1   0
   +---+---+---+---+---+---+---+---+
   |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     |   |   |   |   |   |   |   |
     |   |   |   |   |   +---+---+---> Nro. de int. si bits 5-7 son 111b 0 110b
     |   |   |   +---+---------------> 00 para indicar que se trata de OCW2
     +---+---+-----------------------> Codigo de comando
                                       000 = desactiva la rot. de prior. en modo auto. de EOI
                                       001 = EOI no especifico
                                       010 = indefinido
                                       011 = comando de EOI
                                       100 = activa la rot. de prior. en modo auto. de EOI
                                       101 = rot. de prior. con EOI no especifico
                                       110 = establecer prioridad
                                       111 = rot. prioridades con EOI especifico

   De los muchos comandos que se pueden usar, solo el codigo de comando 001b es relevante.
   Este se usa para indicar la finalizacion de una interrupcion.

   COMUNICACION PIC - MANEJADOR DE INTs.
   -------------------------------------
   Cuando el CPU establece la linea INTA, este le dice al PIC que puede enviar una int. a
   la CPU para su procesamiento. Pero cuando el procesamiento a sido concluido por el 
   manejador, el PIC debe ser notificado antes de la ejecucion de la instruccion IRET.
   
   El manejador de ints. notifica al PIC enviandole un comando OCW2 al primer puerto. El
   codigo de commando, 001b (EOI no especificado). Cuando se lo combina con el resto de los
   bits, el resultado es 0x20h, es decir 00100000b.
   
   Para ints. que son manejadas directamente por el maestro, la rutina en maquina es:

      mov    al, 20h    ; Comando de EOI
      out    20h, al    ; Envia el OCW2 al maestro

   Para las ints. del esclavo.
   
      mov    al, 20h    ; Comando de EOI
      out    0A0h,al    ; Envia el OCW2 al esclavo
      out    20h, al    ; Envia el OCW2 al maestro




*/   

/* ====================================================================== *
 == Constantes                             ==
 ======================================================================== */
#define ICW1_AT         0x11    /* 00010001b igual que el BIOS lo hace */
                                /* constante, en cascada y espera ICW4 */
#define ICW4_AT         0x01    /* terminacion manual de las interrups */

PRIVADA bool aviso_irq(int irq, regs * r);

man_irq  tabla_irq[NRO_IRQS];


/* ====================================================================== *
 ==  completa la tabla de irqs con una funcion tonta            ==
 ======================================================================== */
PUBLICA void inic_pic()
{
   mostrar("\nInicializando PIC");
   outp8(INT_CTL, ICW1_AT);
   
   /* ICW2 para el maestro. Establece la dir. base para las interrupciones */
   outp8(INT_CTLMASK, IRQ_VECTOR0);
   
   /* ICW3 para el maestro. Establece todas la lineas como directas al
   *  hardware menos la IR2 que se utiliza para enlazar a otro PIC en
   *  cascada.
   */
   outp8(INT_CTLMASK, (1 << CASCADE_IRQ));
   
   /* ICW4 para el maestro. Indica que la finalizacion de las Ints. se
   *  deben realizar por software mediante en envio de comandos OCW2
   */
   outp8(INT_CTLMASK, ICW4_AT);
   
   /* Enmascaro todas las Ints. No acepto ninguna */
   outp8(INT_CTLMASK, ~(1 << CASCADE_IRQ));
   
   /* IDEM para el PIC esclavo. */
   outp8(INT2_CTL, ICW1_AT);
   outp8(INT2_CTLMASK, IRQ_VECTOR8);
   outp8(INT2_CTLMASK, CASCADE_IRQ);
   outp8(INT2_CTLMASK, ICW4_AT);
   outp8(INT2_CTLMASK, ~0);
}


/* ====================================================================== *
 ==  completa la tabla de irqs con una funcion tonta            ==
 ======================================================================== */
PUBLICA void inic_irqs()
{
   int   i;

   mostrar("\nInicializando   irqs...");
   for(i=0; i<NRO_IRQS; i++)
      tabla_irq[i] = aviso_irq;
}


/* ====================================================================== *
 ==  la funcion tonta. Muestra el numero de irq que se produjo           ==
 ======================================================================= */
PRIVADA bool aviso_irq(int irq, regs * r)
{
   mostrar("\n\t\tirq %d no esperada...", irq);
   return TRUE;
}

/* ====================================================================== *
 ==  Establece una funcion para manejar una irq especificada y la activa ==
 ======================================================================== */
PUBLICA void est_man_irq(int irq, man_irq manejador)
{
   if(irq < 0 || irq >= NRO_IRQS )
      FATAL("Parametro no valido en est_man_irq()");
   
   tabla_irq[irq] = manejador;
   activar_irq(irq);
}


/* ====================================================================== *
 ==  Activa una IRQ.                                                     ==
 ======================================================================== */
PUBLICA void activar_irq(int irq)
{
   if(irq < 8) outp8(INT_CTLMASK,  inp8(INT_CTLMASK) & ~(1 << irq));
   else        outp8(INT2_CTLMASK, inp8(INT2_CTLMASK) & ~(1 << irq)); 
}


/* ====================================================================== *
 ==  Desactiva una IRQ.                                                  ==
 ======================================================================== */
PUBLICA void desactivar_irq(int irq)
{
   if(irq < 8) outp8(INT_CTLMASK,  inp8(INT_CTLMASK)  | (1 << irq));
   else        outp8(INT2_CTLMASK, inp8(INT2_CTLMASK) | (1 << irq)); 
}

