/*
 * c0.S
 *
 * Punto de entrada al sistema. _start.
 * Inicializo todos los registros de segmento con el descriptor nro. 0x10 (2)
 * correspondiente a los datos.
 *
 */

#include <const.h>
#include <sconst.h>

/* Desplazamiento del puntero a la pila en el anillo 0 del TSS. */
#define TSS_CS		      72
#define TSS_SP0          4
.extern _nucleotss
.extern _cabeza_retenidos
.extern _p_actual

.text
.global _start

/* Punto de entrada en 0x0010000 */
_start:
   cli                        /* limpio la bandera para las interrupciones */
   movl  $0x10,%eax           /* todos los registros de segmento al descriptor */
   movw  %ax,  %es            /* nro. 0x10 (2) */
   movw  %ax,  %ds
   movw  %ax,  %fs
   movw  %ax,  %gs
   movw  %ax,  %ss
   movl  $MEM_PILA, %esp      /* la pila */

   jmp    here
here:

   call  _main                /* vamos al main() del kernel.c */ 
                              /* establezco la bandera para aceptar las ints. */

/* y un bucle infinito que garantiza que no nos vamos de aqui */
aca:
   jmp   aca



/*************************************************************
 * _int_0 - int8                                             *
 ************************************************************/
salvar:
   pushal          /* guardamos todos los regs. generales  */
   cld             /* bit de direccion a un valor conocido */
   pushl %ds       /* guardamos el resto */
   pushl %es
   pushl %fs
   pushl %gs

   /* todos los registros de segmento iguales a SS para impidir que 
    * un manejador de ints. pueda acceder al espacio de usr.
    */
   movw  %ss,  %dx
   movw  %dx,  %ds
   movw  %dx,  %es

   /* guardo el puntero a la pila (esp) */
   movl  %esp, %eax 

   /* la variable _reentrando tiene un valor igual a -1 cuando no se esta reentrando
    * al kernel y > -1 si se esta reentrando. Por eso, si luego de incrementar la 
    * variable _reentrando, esta tiene un valor distinto de cero, significa que ya 
    * estamos en el kernel y que por o tanto, la pila que estamos usando es la del
    * kernel por lo que no debemos hacer nada mas, mientras que si es la primera vez 
    * que entramos, debemos establecer el puntero a la pila 'esp' al tope de la pila 
    * del kernel.
    */
   incb  _adentro
   jnz   est_ret_ya_estamos
   
   movl  $MEM_PILA, %esp 
   pushl $no_estabamos
   jmp   DIR_RET(%eax)

est_ret_ya_estamos:
   pushl $ya_estabamos
   jmp   DIR_RET(%eax)


no_estabamos:
   cmpl  $0, (_cabeza_retenidos)           /* if(retenidos == NULL) */
   jz    no_retenidas
   call _lanzar_ints

no_retenidas:
   movl  (_p_actual), %eax        /* apuntamos a los registros de la tabla de proc. */
   ltr   TSS_CS(%eax) 
   jmp   ya_estabamos

.global _girar
_girar:
   sti

ya_estabamos:
   decb  _adentro
   popl  %gs
   popl  %fs
   popl  %es
   popl  %ds

   popal
   add   $4,   %esp
   iret





/*************************************************************
 * int_sistema
 ************************************************************/
.global _int_sistema
_int_sistema:
   pushl %ebx    /* ptr. mensaje   */
   pushl %ecx    /* pid del proceso destino u origen */
   pushl %eax    /* ENVIAR y/o RECIBIR   */
   call  _llamada_sistema
   addl  $12,  %esp
   iret



/*************************************************************
 * int_00 - int_07
 ************************************************************/
.macro int_irq nro
   call  salvar

   movb  $ACTIVADO, %al
   outb  %al, $INT_CTL

   pushl 0x0           /* codigo */
/* sti */
   lea   (%esp), %eax
   pushl %eax
   pushl $\nro
   call  *(_tabla_irq + 4*\nro)  /* tabla_irq[nro_irq](nro_irq, regs *); */
   popl  %ecx
   popl  %ecx
   popl  %ecx

/* cli */
   test  %eax, %eax
   jz 0f

.if \nro<8 
   inb   $INT_CTLMASK, %al          
   andb  $~(1<<\nro), %al           
   outb  %al,  $INT_CTLMASK          
.else                               
   inb   $INT2_CTLMASK, %al         
   andb  $~(1<<(\nro-8)), %al       
   outb  %al,  $INT2_CTLMASK         
.endif
0:       ret
.endm

/*************************************************************
 * int_00 - int_07
 ************************************************************/
.global _int_00, _int_01, _int_02, _int_03
.global _int_04, _int_05, _int_06, _int_07

   .align 4
_int_00:  int_irq 0

   .align 4
_int_01:  int_irq 1

   .align 4
_int_02:  int_irq 2

   .align 4
_int_03:  int_irq 3

   .align 4
_int_04:  int_irq 4

   .align 4
_int_05:  int_irq 5

   .align 4
_int_06:  int_irq 6

   .align 4
_int_07:  int_irq 7

/*************************************************************
 * int_08 - int_15
 *************************************************************/
.global _int_08, _int_09, _int_10, _int_11
.global _int_12, _int_13, _int_14, _int_15

   .align 4
_int_08:  int_irq   8
                      
   .align 4           
_int_09:  int_irq    9
                      
   .align 4           
_int_10:  int_irq   10
                      
   .align 4           
_int_11:  int_irq   11
                      
   .align 4           
_int_12:  int_irq   12
                      
   .align 4           
_int_13:  int_irq   13
                      
   .align 4           
_int_14:  int_irq   14
                      
   .align 4           
_int_15:  int_irq   15

/*************************************************************
 * excepciones
 ************************************************************/
excepcion_s_err: 
   movl  $0x0, (_cod_error)
   popl  (_nro_exc)
   jmp   excepcion

excepcion_c_err: 
   popl  (_nro_exc)
   popl  (_cod_error)

excepcion:
   call  salvar         /* Salva absolutamente todo */
   pushl (_cod_error)   /* */
   lea   (%esp), %eax   /* eax = dir regs */
   pushl %eax           /* empuja regs */
   pushl (_nro_exc)
   call  _manejador     /* manejador(nro_exc, regs*)*/
   popl  %eax
   ret

/*************************************************************
 * excepciones
 *************************************************************/
   .align 4
   .global _division_cero
_division_cero:
   pushl $0
   jmp   excepcion_s_err

   .align 4
   .global _paso_a_paso
_paso_a_paso:
   pushl $1
   jmp   excepcion_s_err


   .align 4
   .global _inem
_inem:
   pushl $2
   jmp   excepcion_s_err

   .align 4
   .global _break_point
_break_point:
   pushl $3
   jmp   excepcion_s_err

   .align 4
   .global _sobreflujo
_sobreflujo:
   pushl $4
   jmp   excepcion_s_err

   .align 4
   .global _limite
_limite:
   pushl $5
   jmp   excepcion_s_err

   .align 4
   .global _op_no_valida 
_op_no_valida:
   pushl $6
   jmp   excepcion_s_err

   .align 4
   .global _cp_no_disp
_cp_no_disp:
   pushl $7
   jmp   excepcion_s_err

   .align 4
   .global _doble_falla
_doble_falla:
   pushl $8
   jmp   excepcion_c_err

   .align 4
   .global _cp_seg
_cp_seg:
   pushl $9
   jmp   excepcion_s_err

   .align 4
   .global _tss_ausente
_tss_ausente:
   pushl $10
   jmp   excepcion_c_err

   .align 4
   .global _seg_ausente
_seg_ausente:
   pushl $11
   jmp   excepcion_c_err

   .align 4
   .global _exc_pila
_exc_pila:
   pushl $12
   jmp   excepcion_c_err

   .align 4
   .global _prot_gral
_prot_gral: 
   pushl $13
   jmp   excepcion_c_err

   .align 4
   .global _fallo_pagina
_fallo_pagina:
   pushl $14
   jmp   excepcion_c_err

   .align 4
   .global _cp_error
_cp_error:
   pushl $15
   jmp   excepcion_s_err



_cod_error: .long   0
_nro_exc:   .long   0

.global _adentro
_adentro:   .byte   0

_ktop:      .long   0
