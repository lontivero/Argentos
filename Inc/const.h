/***********************************************************************************************
*   const.h
***********************************************************************************************/
#ifndef _MSO_CONST_H_
#define _MSO_CONST_H_

#define         INT_VIDEO               0x10
#define         INT_KEYBOARD            0x16
#    define     WAIT_FOR_KEYSTROKE      00

#define         INT_DISK                0x13
#    define     READ_SECTOR             0x0201
#    define     GET_STATUS              0x0100
#    define     RESET                   0x0000

#define         INT_DOS                 0x21
#    define     ALLOC_MEMORY            0x4800


/* Generales */
#define NULL	((void *)0)
#define _1KB_	1024
#define _2KB_	(2 * _1KB_)
#define _4KB_	(4 * _1KB_)
#define _1MB_	(_1KB_ * _1KB_)
#define _4MB_	(4 * _1MB_)




/* Para el Adm. memoria */
#define TAM_PAGINA	_4KB_

#define CMOS_PL	  0x70
#define CMOS_PE     0x71

#define BYTE(b)	 (b & 0xff)

#define PUBLICA
#define PRIVADA	static 
#define EXTERNA   extern

#ifdef DBG
#	define vd		mostrar
#endif


/* Algunos Puertos */
#define RELOJ0          0x40    
#define RELOJ2          0x42    
#define MODO_RELOJ      0x43    
#define FREQ_RELOJ  	1193181    	/* senales por segundo del i8254 */
#define ONDA_CUADRADA   0x36    
                       

/* Direcciones de memoria y tamaños */
#define MEM_CODIGO		0x0010000
#define MEM_BLOQUE0		0x0007C00
#define MEM_MONTON		0x0020000
#define MEM_VIDEO			0x00b8000
#define MEM_PILA			0x000ffc0

#define TAM_BLOQUE0		0x0000600
#define TAM_MONTON		0x0010000
#define TAM_VIDEO			0x0010000
#define TAM_PILA			0x00ffc0

#define FIN_BLOQUE0		(MEM_BLOQUE0 + TAM_BLOQUE0)
#define FIN_MONTON		(MEM_MONTON  + TAM_MONTON)	
#define FIN_VIDEO			(MEM_VIDEO   + TAM_VIDEO)	

/* ====================================================================== *
 == Constantes										 ==
 ======================================================================== */
#define INT_CTL		0x20
#define INT_CTLMASK	0x21
#define INT2_CTL		0xA0
#define INT2_CTLMASK	0xA1
#define CASCADE_IRQ	0x02
#define ACTIVADO		0x20

/* ====================================================================== *
 == Constantes										 ==
 ======================================================================== */
#define NRO_IRQS 15

#define INT_SISTEMA     0x2F
#define IRQ_VECTOR0	0x30
#define IRQ_VECTOR8	(IRQ_VECTOR0 + 8)

#define IRQ_VEC(vec)	(IRQ_VECTOR0+vec)


#define IRQ_RELOJ		0
#define IRQ_TECLADO	1
#define IRQ_FLOPPY	6

#endif

