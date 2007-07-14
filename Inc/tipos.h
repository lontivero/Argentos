/***********************************************************************************************
*   tipos.h
************************************************************************************************
*   Declaracion de tipos basicos.
*      - intXX		entero con signo de XX bits.
*      - untXX		entero sin signo de XX bits.
**********************************************************************************************/
#ifndef _MSO_TIPOS_H_
#define _MSO_TIPOS_H_

typedef enum {FALSE, TRUE} bool;

typedef char	int8;
typedef short	int16;
typedef int		int32;
typedef long	int64;

typedef char	unsigned unt8;
typedef short	unsigned unt16;
typedef int		unsigned unt32;
typedef long	unsigned unt64;

typedef unt32	dir32;
typedef unt16	selector;

typedef unt32	r386g;
typedef unt16	r386s;

typedef int		cod_err;	

#define __attribute__(x) 
#define __inline__(x)
typedef struct 
{
   unt32    codigo  __attribute__((packed));
   r386s	gs  __attribute__((packed));
   r386s    no_usado1  __attribute__((packed));
   r386s	fs  __attribute__((packed));
   r386s    no_usado2  __attribute__((packed));
   r386s	es  __attribute__((packed));
   r386s    no_usado3  __attribute__((packed));
   r386s	ds  __attribute__((packed));
   r386s    no_usado4  __attribute__((packed));
   r386g    edi  __attribute__((packed));
   r386g    esi  __attribute__((packed));
   r386g	ebp  __attribute__((packed));
   r386g    xesp __attribute__((packed));   /* este lo pone el pushad */
   r386g    ebx  __attribute__((packed));
   r386g    edx  __attribute__((packed));
   r386g    ecx  __attribute__((packed));
   r386g    eax  __attribute__((packed));

   /* salvar() pone aqui la dir a retornar que puede ser ya_estabamos
    * o mo_estabamos.
    */
   r386g    ret  __attribute__((packed));

   /* estos los pone la CPU durante una interrupcion */
   r386g    eip  __attribute__((packed));
   r386s	cs  __attribute__((packed));
   r386s    no_usado5  __attribute__((packed));
   r386g    eflags  __attribute__((packed));
   r386g    esp  __attribute__((packed));
   r386s	ss  __attribute__((packed));
   r386s    no_usado6  __attribute__((packed));
} regs __attribute__((packed));

typedef bool (*man_irq)(int irq, regs * r);


#endif
