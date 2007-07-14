/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias el nucleo.                       ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#ifndef _MSO_TECLADO_H_
#define _MSO_TECLADO_H_

#include <tipos.h>
#include "terminal.h"
#include "..\irq.h"

/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define MAX_INTENTO     4
#define P_DATO       0x60
#define P_ESTADO     0x64


#define BIT_BUFF_OUT    1
#define BIT_BUFF_IN     2

#define COM_PAUSA    0x34
#define TXS8          0x0f     /* 8 teclas por segundo */

#define COM_LEDS     0xed
#define BIT_SCROLL      1
#define BIT_NUM         2
#define BIT_CAPS        3

/* Constantes para 'estado'  */
#define ESHIFT       0x01
#define ECTRL        0x02
#define EALT         0x04
#define ECAPS        0x08
#define ENUM         0x10
#define ESCROLL      0x20


#define BIT_SOLTAR   0x80
#define MAX_NUM_TECLAS 128

#define NRO_TECLAS_RAPIDAS 12


/* extended keycode             */
#define HASCAPS     0x8000     /* Caps Lock has effect         */
#define EXT         0x0100     /* Normal function keys         */
#define CTRL        0x0200     /* Control key                  */
#define SHIFT       0x0400     /* Shift key                    */
#define ALT         0x0800     /* Alternate key                */
#define EXTKEY      0x1000     /* extended keycode             */
#define HASCAPS     0x8000     /* Caps Lock has effect         */

#define ESC             27
#define ENTER           13

/* Numeric keypad */
#define HOME     ( 1 + EXT)
#define END      ( 2 + EXT)
#define ARRIBA   ( 3 + EXT)
#define ABAJO    ( 4 + EXT)
#define IZQ      ( 5 + EXT)
#define DER      ( 6 + EXT)
#define PARRIBA  ( 7 + EXT)
#define PABAJO   ( 8 + EXT)
#define MEDIO    ( 9 + EXT)
#define MENOSP   (10 + EXT)
#define MASP     (11 + EXT)
#define INSERTAR (12 + EXT)

/* Alt + Numeric keypad */
#define AHOME    ( 1 + ALT)
#define AEND     ( 2 + ALT)
#define AARRIBA  ( 3 + ALT)
#define AABAJO   ( 4 + ALT)
#define AIZQ     ( 5 + ALT)
#define ADER     ( 6 + ALT)
#define APARRIBA ( 7 + ALT)
#define APABAJO  ( 8 + ALT)
#define AMEDIO   ( 9 + ALT)
#define AMENOSP  (10 + ALT)
#define AMASP    (11 + ALT)
#define AINSERTAR (12 + ALT)

/* Ctrl + Numeric keypad */
#define CHOME    ( 1 + CTRL)
#define CEND     ( 2 + CTRL)
#define CARRIBA  ( 3 + CTRL)
#define CABAJO   ( 4 + CTRL)
#define CIZQ     ( 5 + CTRL)
#define CDER     ( 6 + CTRL)
#define CPARRIBA ( 7 + CTRL)
#define CPABAJO  ( 8 + CTRL)
#define CMEDIO   ( 9 + CTRL)
#define CMENOSP  (10 + CTRL)
#define CMASP    (11 + CTRL)
#define CINSERTAR (12 + CTRL)


/* Lock keys */
#define CAPS     (13 + EXT)    /* caps lock    */
#define NUM      (14 + EXT)    /* number lock  */
#define SCROLL   (15 + EXT)    /* scroll lock  */

/* Function keys */
#define F1       (16 + EXT)
#define F2       (17 + EXT)
#define F3       (18 + EXT)
#define F4       (19 + EXT)
#define F5       (20 + EXT)
#define F6       (21 + EXT)
#define F7       (22 + EXT)
#define F8       (23 + EXT)
#define F9       (24 + EXT)
#define F10      (25 + EXT)
#define F11      (26 + EXT)
#define F12      (27 + EXT)

/* Alt+Fn */
#define AF1      (16 + ALT)
#define AF2      (17 + ALT)
#define AF3      (18 + ALT)
#define AF4      (19 + ALT)
#define AF5      (20 + ALT)
#define AF6      (21 + ALT)
#define AF7      (22 + ALT)
#define AF8      (23 + ALT)
#define AF9      (24 + ALT)
#define AF10     (25 + ALT)
#define AF11     (26 + ALT)
#define AF12     (27 + ALT)

/* Ctrl+Fn */
#define CF1      (16 + CTRL)
#define CF2      (17 + CTRL)
#define CF3      (18 + CTRL)
#define CF4      (19 + CTRL)
#define CF5      (20 + CTRL)
#define CF6      (21 + CTRL)
#define CF7      (22 + CTRL)
#define CF8      (23 + CTRL)
#define CF9      (24 + CTRL)
#define CF10     (25 + CTRL)
#define CF11     (26 + CTRL)
#define CF12     (27 + CTRL)
	
/* Shift+Fn */
#define SF1      (0x10 + SHIFT)
#define SF2      (0x11 + SHIFT)
#define SF3      (0x12 + SHIFT)
#define SF4      (0x13 + SHIFT)
#define SF5      (0x14 + SHIFT)
#define SF6      (0x15 + SHIFT)
#define SF7      (0x16 + SHIFT)
#define SF8      (0x17 + SHIFT)
#define SF9      (0x18 + SHIFT)
#define SF10     (0x19 + SHIFT)
#define SF11     (0x1A + SHIFT)
#define SF12     (0x1B + SHIFT)

#define S(x)     (x|SHIFT)
#define C(x)     (x|CTRL)
#define A(x)     (x|ALT)
#define CA(x)    C(A(x))




/* ====================================================================== *
 == Estructuras                                                          ==
 ======================================================================== */
typedef void (*fnc_tecla)();


typedef struct {
   unt8 codigo;
   fnc_tecla fnc;
}  tecla_rapida;


typedef struct{
   unt16 normal;
   unt16 shift;
   unt16 ctrl;
   unt16 alt;
   unt16 ctrlalt;
}  mapa_teclado;


/* ====================================================================== *
 == Funciones                                                            ==
 ======================================================================== */
PUBLICA void leer_teclado(terminal *t);
PUBLICA bool irq_teclado(int irq, regs * r);
PUBLICA void inic_teclado();

#endif
