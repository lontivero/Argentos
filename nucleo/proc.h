/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo de los procesos.  ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                           .==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */
#ifndef _MSO_PROC_H_
#define _MSO_PROC_H_

#include <const.h>
#include <tipos.h>



/* ====================================================================== *
 == Constantes                                                           ==
 ======================================================================== */
#define  NRO_PROCS    20
                        
#define  LIBRE         1
#define  ENVIANDO      2
#define  RECIBIENDO    4
#define  PENDIENTE     8
#define  LISTO    0     
                        
/* Funcion        */
#define  ENVIAR        1
#define  RECIBIR       2
#define  ENV_REC       3


/* A QUIEN        */
#define  HARDWARE     -2
#define  CUALQUIERA   -1
#define  T_OCIOSA      0
#define  T_TERM        1
#define  T_RELOJ       2
#define  T_MEM_FISICA  3
#define  T_DRAM        4
#define  T_FLOPPY      5
#define  T_TEST        6

/* #### ANULADOS #### */
//#define  T_SA      6


#  define  ULTIMA_TAREA    T_FLOPPY
#  define  ULTIMO_SERVIDOR T_FLOPPY
#  define  ULTIMO_PROCESO  T_TEST

#define  NRO_TAREAS      ULTIMA_TAREA + 1
#define  NRO_SERVIDORES (ULTIMO_SERVIDOR - ULTIMA_TAREA)
#define  NRO_PROCESOS   (ULTIMO_PROCESO  - ULTIMO_SERVIDOR)
#define  NRO_PROCESOS_TOTAL (ULTIMO_PROCESO + 1)

#define  es_tarea(n)    (n<=ULTIMA_TAREA)
#define  es_servidor(n) (n<=ULTIMO_SERVIDOR && n>ULTIMA_TAREA)
#define  es_proceso(n)  (n<=ULTIMO_PROCESO  && n>ULTIMO_SERVIDOR)

#define  DIR_INI_PROC   (&tabla_proc[0])
#define  DIR_ULT_PROC   (&tabla_proc[NRO_PROCS])



/* retornar       */
#define OK                 0
#define E_BLOQUEO_MUTUO   -1
#define E_PARAMETRO       -2
#define E_OP_INV          -3
#define E_INV             -4
#define E_SIN_DISP        -5
#define E_SIN_IOCTL       -6
#define E_ES              -7
#define E_SERVICIO        -8


#define  FLAGS_INICIAL 0x202

/* Segmentos de un proceso */
#define  NRO_SEGMENTOS     3                 /* Texto, Datos y Pila  */
/* Indices dentro de la tabla de descriptores locales */
#define  T           0                 /* Texto */
#define  D           1                 /* Datos */
#define  P           2                 /* Pila  */


/* Tamano de las pilas para las tareas */
#define BLOQUE_MIN   0x200       /* tarea simple (512 bytes = 256 words = 128 dwords)*/
#define BLOQUE_MED   2 * BLOQUE_MIN /* tarea pequena  (1 kbyte) */
#define BLOQUE_MAX   4 * BLOQUE_MED /* tarea compleja (4 kbyte) */

#define PILA_OCIOSA  BLOQUE_MIN
#define PILA_TERM    BLOQUE_MED
#define PILA_RELOJ   BLOQUE_MED
#define PILA_MFISICA BLOQUE_MAX
#define PILA_DRAM    BLOQUE_MED
#define PILA_FLOPPY  BLOQUE_MED
#define PILA_SISTEMA BLOQUE_MED
#define PILA_SA      BLOQUE_MAX


#define PILA_TEST    BLOQUE_MIN

#define PILA_TOTAL   \
           PILA_OCIOSA + PILA_RELOJ + PILA_MFISICA + PILA_DRAM + \
           PILA_SISTEMA + PILA_SA + PILA_FLOPPY + \
            PILA_TERM + 3*PILA_TEST


/* ====================================================================== *
 == Tipos                                                                ==
 ======================================================================== */
typedef int    pid;

typedef struct {
   pid   quien;
   unt16 codigo;
   
   unt32 num32_1, num32_2, num32_3;
   unt16 num16_1, num16_2;
}  mensaje;


/* descriptor */
typedef struct
{
   unt16 limite00_15    __attribute__((packed));
   unt16 base00_15      __attribute__((packed));
   unt8  base16_23      __attribute__((packed));
   unt8  tipo           __attribute__((packed));      /* |P|DL|1|X|E|R|A| */     
   unt8  limite16_23    __attribute__((packed));
   unt8  base24_31      __attribute__((packed));
} __attribute__((packed)) descriptor;



/* TSS  */ 
typedef struct 
{
   unt16 back __attribute__((packed));       // Backlink field
   unt16 nu0  __attribute__((packed));        // all nu* are nuerved and set to 0
   unt32 esp0 __attribute__((packed));       // Ring 0 ESP
   unt16 ss0  __attribute__((packed));        // Ring 0 SS
   unt16 nu1  __attribute__((packed));
   unt32 esp1 __attribute__((packed));       // Ring 1 ESP
   unt16 ss1  __attribute__((packed));        // Ring 1 SS
   unt16 nu2  __attribute__((packed));
   unt32 esp2 __attribute__((packed));       // Ring 2 ESP
   unt16 ss2  __attribute__((packed));        // Ring 2 SS
   unt16 nu3  __attribute__((packed));
   unt32 cr3  __attribute__((packed));        // CR3 (Page Direcory Base Register)

   unt32 eip __attribute__((packed));
   unt32 eflags __attribute__((packed));
   unt32 eax __attribute__((packed));
   unt32 ecx __attribute__((packed));
   unt32 edx __attribute__((packed));
   unt32 ebx __attribute__((packed));
   unt32 esp __attribute__((packed));
   unt32 ebp __attribute__((packed));
   unt32 esi __attribute__((packed));
   unt32 edi __attribute__((packed));

   unt16 es  __attribute__((packed));
   unt16 nu4 __attribute__((packed));
   unt16 cs  __attribute__((packed));
   unt16 nu5 __attribute__((packed));
   unt16 ss  __attribute__((packed));
   unt16 nu6 __attribute__((packed));
   unt16 ds  __attribute__((packed));
   unt16 nu7 __attribute__((packed));
   unt16 fs  __attribute__((packed));
   unt16 nu8 __attribute__((packed));
   unt16 gs  __attribute__((packed));
   unt16 nu9 __attribute__((packed));

   unt16 ldt   __attribute__((packed));      // LDT descriptor
   unt16 nu10  __attribute__((packed));
   unt16 trap  __attribute__((packed));      // bit 0 indicates trap-on-task-switch
   unt16 iomap __attribute__((packed));      // Offset of I/O Map into tss
}  tss __attribute__((packed));




/* mapa de memoria del proceso */
typedef struct
{
   dir32 base;
   unt32 tam;
}  map;

/* ====================================================================== *
 == tipos                                                                ==
 ======================================================================== */
typedef struct proc{
   tss    p_tss;
   selector selector_tss;

   pid    nro;
   char   nombre[20];
   unt16  estado;
   
   struct proc  *prox;
   struct proc  *padre;
   
   selector   selector_ldt;
   descriptor ldt[NRO_SEGMENTOS];

   map    mapa[3];
   struct proc  *primer_llamador;   /* los que me estan esperando */ 
   struct proc  *prox_llamador;
   
   
   int int_retenida;
   struct proc  *p_prox_retenido;
   
   
   mensaje * pmensaje;        /* el mensaje  */
   pid       recibiendo_de;   /* nro proc. de quien espero recibir noticias */
   pid       enviando_a;      /* nro proc. a quien quiero enviar algo   */
   bool      esperando_int;
}  proc;



/* ====================================================================== *
 == prototipos de funciones                                              ==
 ======================================================================== */
PUBLICA int  penviar(proc *origen, pid destino, mensaje * pm);
PUBLICA int  precibir(proc * llamador, pid origen, mensaje * pm);
PUBLICA void inic_procs();
PUBLICA void cambiar(bool girar);
PUBLICA void interrupcion(pid tarea);
PUBLICA void asignar_segmentos(proc * p_proc);


#define recibir(nro, men)  precibir(p_actual, nro, men)
#define enviar(nro, men)   penviar(p_actual, nro, men)
#define envrec(nro, men)   enviar(nro, men); recibir(nro, men);

proc *p_actual;         /* el proceso en ejecucion */

extern struct proc *cabeza_retenido;  
extern struct proc *cola_retenido;    
extern int    reentrando;             


/* ====================================================================== *
 == Las tareas                                                           ==
 ======================================================================== */
typedef struct {
   char  nombre[32];
   unt32 tam_pila;
   void  (*rutina)();
} tarea;

extern void t_ociosa();
extern void t_reloj();
extern void t_terminal();
extern void t_debug();
extern void t_test();
extern void t_cdf();


#endif

