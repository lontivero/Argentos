/* ====================================================================== *
 ==  Este archivo contiene las funciones para el manejo del FDC          ==
 ==  (Controlador de Discos Flexibles)                                   ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <mem.h>
#include <mensajes.h>
#include <macros.h>
#include "driver.h"
#include "cdf.h"
#include "consola.h"
#include "..\proc.h"
#include "..\irq.h"
#include "..\otros.h"


/* Puertos  */
#define DMA_READ     0x44
#define DMA_WRITE    0x48
#define RTCSEL       0x70
#define RTCDATA      0x71

/* Puertos  del FDC */
#define FD_MSR       0x3f4    /* Main Status Register       */
#define FD_DATA      0x3f5    /* Data (FIFO)          */
#define FD_DOR       0x3f2    /* Digital Output Register       */
#define FD_CCR       0x3f7    /* Configuaration Control Register (write)*/

/* Bits para MSR */
#define STATUS_BUSY  0x10     /* FDC ocupado          */
#define STATUS_DIR   0x40     /* 0: escribir a FDC 1: leer de FDC */
#define STATUS_READY 0x80     /* Reg. de datos listo        */

/* Bits para DOR */
#define FD_MOTOR0    0x10
#define ACTIVAR_INT  0x0C


/* Registro de estado retornado como resultado de una operacion.   */
#define ST_CYL       0x03     /* entrada donde retorta el cilindro */
#define ST_HEAD      0x04     /* entrada donde retorta la cabeza   */
#define ST_SEC       0x05     /* entrada donde retorta el sector   */

/* ST0. */
#define ST0_BITS     0xF8     /* check top 5 bits of seek status */
#define TRANS_ST0    0x00     /* top 5 bits of ST0 for READ/WRITE */
#define SEEK_ST0     0x20     /* top 5 bits of ST0 for SEEK */
	
/* ST1. */
#define BAD_SECTOR   0x05     /* if these bits are set in ST1, recalibrate */
#define WRITE_PROTECT 0x02    /* bit is set if diskette is write protected */
	
/* ST2. */
#define BAD_CYL      0x1F     /* if any of these bits are set, recalibrate */
	
/* ST3 (not used). */
#define ST3_FAULT       0x80  /* if this bit is set, drive is sick */
#define ST3_WR_PROTECT  0x40  /* set when diskette is write protected */
#define ST3_READY       0x20  /* set when drive is ready */


#define FD_RECALIBRAR   0x07  /* ir a cilindro 0         */
#define FD_SEEK         0x0F  /* ir a un cilindro        */
#define FD_READ         0xE6  /* leer con MT, MFM, salta borrado  */
#define FD_WRITE        0xC5  /* escribir con MT, MFM       */
#define FD_SENSE        0x08  /* Sense Interrupt Status     */
#define FD_SPECIFY      0x03  /* specify HUT etc         */
#define FD_VERSION      0x10
#  define FDTIPO_NONE   0x00
#  define FDTIPO_360    0x01
#  define FDTIPO_1200   0x02
#  define FDTIPO_720    0x03
#  define FDTIPO_1440   0x04
#  define FDTIPO_2880   0x05

#  define FD_VER_765A   0x80
#  define FD_VER_765B   0x90


/* Errores */
#define E_SEEK         -1
#define E_RECALIBRAR   -2
#define E_TIMEOUT      -3
#define E_ESTADO       -4

/* Geometria */
#define NRO_CABEZAS     2
#define NRO_SECTORES    2880
#define NRO_CILINDROS   18
#define NRO_SECTORES_PISTA 80

/* Tiempos de espera en ticks de reloj */
#define TIEMPO_ESPERA   200   /* espera 200 ticks antes de declarar al FDC know-out */
#define TIEMPO_APAGADO  400   /* espera 400 ticks antes de apagar el motor */


/* Para FD_READ y FD_WRITE */
#define DTL             0xff
#define GAP             0x1b
#define S512            2


/* Iteraciones */
#define NRO_INTENTOS 3
#define MAX_NRO_PRUEBAS 10000

#define MAX_ESTADOS  7
unt8 estados[MAX_ESTADOS];
#define ST0 (estados[0])
#define ST1 (estados[1])
#define ST2 (estados[2])
#define ST3 (estados[0])
#define ST_PCN (estados[1])   /* Present Cylinder Number */

PRIVADA dispositivo  geometria;
   
PRIVADA int ocupado;
PRIVADA bool calibrado    = FALSE;
PRIVADA bool prendido     = FALSE;
PRIVADA bool no_responde  = FALSE;
PRIVADA int nuevo_estado  = FALSE;     /* esto se usa para determinar si se debe apagar o no */
                                       /* un motor cuando el reloj llama a motor_off.     */

PRIVADA int  cil_act;         /* cilindro actual */
PRIVADA int  cil_real;        /* cilindro real (hardware)*/
PRIVADA int  cilindro;        /* cilindros */
PRIVADA int  sector;
PRIVADA int  cabeza;


PRIVADA void motor_on();
PRIVADA void motor_off();
PRIVADA void out_fd(unt8 val);
PRIVADA int recalibrar();
PRIVADA void resetear_fd();
PRIVADA void tiempo_cumplido();
PRIVADA int esperar_int();
PRIVADA int estados_fd();
PRIVADA int seek_fd();
PRIVADA int operar(pid proc, operacion_es *es);
PRIVADA void inic_cdf();
PRIVADA bool irq_floppy(int irq, regs * r);
PRIVADA void geometria_floppy(particion * entry);
PRIVADA dispositivo *preparar(int d);
PRIVADA int finalizar();
PRIVADA void apagar_motor();
PRIVADA void com_reloj(int ticks, void (*f)());
PRIVADA int wr_fd(bool ecribo);
PRIVADA void informar_floppy(unt8 drv);



PRIVADA driver floppy_drv = {
   nombre,
   sin_op,          /* abrir    */
   sin_op,          /* cerrar   */
   sin_op,          /* ioctrl   */
   preparar,        /* preparar */
   operar,          /* operara  */
   finalizar,       /* finalizar*/
   sin_limpieza,    /* limpieza */
   geometria_floppy /* geometria*/
};



/* ========================================================================== *
 ==  t_floppy                                                                ==
 * ========================================================================== */
PUBLICA void t_floppy()
{
   inic_cdf();
   t_driver(&floppy_drv);
}


/* ========================================================================== *
 ==  Establece el manejador de INT para la comunicacion de FDC.              ==
 ==  Resetea el controlador.                                                 ==
 ==  Informa sobre la version del controlador                                ==
 * ========================================================================== */
PRIVADA void inic_cdf()
{
   mostrar("\nInicializando FDC...");
   est_man_irq(IRQ_FLOPPY, irq_floppy);

   resetear_fd();

   out_fd(FD_VERSION);
   estados_fd();

   if( ST0 == FD_VER_765A )       mostrar("\n\tFDC estandar encontrado");
   else if( ST0 == FD_VER_765B )  mostrar("\n\tFDC extendido encontrado");
   else if( ST0 == 0 )            mostrar("\n\tFDC no responde");
   else                           mostrar("\n\tcontrolador desconocido");

   informar_floppy(0);
   informar_floppy(1);
}


PRIVADA void informar_floppy(unt8 drv)
{
   mostrar("\n\tFloppy %s :: disquetera de ", (drv == 0) ? "A" : "B");
   switch(obt_tipo_floppy(drv)) {
      case FDTIPO_NONE:   mostrar("<NINGUNA>"); break;
      case FDTIPO_360 :   mostrar("360 kb");    break;
      case FDTIPO_720 :   mostrar("720 kb");    break;
      case FDTIPO_1200:   mostrar("1.2MB");     break;
      case FDTIPO_1440:   mostrar("1.44MB");    break;
      case FDTIPO_2880:   mostrar("2.88MB");    break;
   }
}

/* ====================================================================== *
 == Avisa a motor_on que el motor ya tuvo tiempo de tomar velocidad      ==
 ======================================================================== */
PRIVADA void desbloquear()
{
   interrupcion(T_FLOPPY);
}

/* ====================================================================== *
 == Enciende el motor de la disquetera y espera a que tome velocidad     ==
 ======================================================================== */
PRIVADA void motor_on()
{
   mensaje m;

   /* encendelo */
   outp8(FD_DOR, FD_MOTOR0 | ACTIVAR_INT);

   /* si ya esta encendido, no espero a que tome velocidad :) */
   if(prendido) return;
   prendido = nuevo_estado = TRUE;

   com_reloj(100, desbloquear);
   recibir(HARDWARE, &m);
}


/* ====================================================================== *
 == Apago el motor.                                                      ==
 ======================================================================== */
PRIVADA void motor_off()
{
   /* si no debo apagarlo. no lo apago :(                                 */
   /* esto ocurre cuando el reloj llama a esta funcion pero una nueva     */
   /* operacion esta en tramite por lo que el motor debe seguir encendido.*/
   if(nuevo_estado == prendido) return;

   prendido = nuevo_estado = FALSE;
   outp8(FD_DOR, ACTIVAR_INT);
}


/* ====================================================================== *
 == Hace que el reloj apage el motor despues de TIEMPO_APAGADO           ==
 == por si otra operacion comienza inmediatamente                        ==
 ======================================================================== */
PRIVADA void apagar_motor()
{
   nuevo_estado = FALSE;
   com_reloj(TIEMPO_APAGADO, motor_off);
}

/* ====================================================================== *
 == Cada vez que el FDC termina una operacion genera una INT.            ==
 == Entonces, cada vez que enviamos un comando debemos bloquearnos a la  ==
 == espera de que termine por lo que cuando esta INT llega debemos       ==
 == desbloquear el proceso. Esto lo hacemos enviando un mensaje.         ==
 ======================================================================== */
PRIVADA bool irq_floppy(int irq, regs * r)
{
   interrupcion(T_FLOPPY);
   return FALSE;
}



/* ====================================================================== *
 == Envia comandos al FDC. Para enviar un byte el controlador debe estar ==
 == listo para recibir. Por eso probamos un numero de veces y si despues ==
 == de estos intentos todavia no esta listo, hay que resetear el FDC.    ==
 ======================================================================== */
PRIVADA void out_fd(unt8 val)
{
   int nro_pruebas;

   if (no_responde) return;
   
   nro_pruebas = MAX_NRO_PRUEBAS;
   while ((inp8(FD_MSR) & (STATUS_READY | STATUS_DIR)) != STATUS_READY)
      if (nro_pruebas-- <= 0)
      {
         no_responde = TRUE;
         return;
      }

   outp8(FD_DATA, val);
}





#define APARCADO       (-1)
/* ====================================================================== *
 == Envia comandos al FDC. Para enviar un byte el controlador debe estar ==
 == listo para recibir. Por eso probamos un numero de veces y si despues ==
 == de estos intentos todavia no esta listo, hay que resetear el FDC.    ==
 ======================================================================== */
PRIVADA int recalibrar()
{
   int r;
   
   motor_on();                /* Para recalibrar el motor debe estar encendido */
   out_fd(FD_RECALIBRAR);     /* Enviamos el comando */
   out_fd(0);                 /* drive (0 porque es el unico)*/
   if (no_responde) return E_SEEK; 
   
   /* Esperamos a que finalice el comando */
   if (esperar_int() != OK) return E_TIMEOUT; 
	
   /* La calibracion se realizo con exito */
   out_fd(FD_SENSE);
   r = estados_fd(); 
   cil_act = APARCADO;
   if(r != OK  || (ST0 & ST0_BITS) != SEEK_ST0 || ST_PCN != 0)
   {
      no_responde = TRUE;
      return E_RECALIBRAR;
   }
  
   calibrado = TRUE;
   return OK; 
}



/* ====================================================================== *
 == Resetea el controlador.   Esto se hace cuando el controlador no      ==
 == responde aunque no se muy bien por que se da esta situacion llamada  ==
 == "catastrofe".                                                        ==
 ======================================================================== */
PRIVADA void resetear_fd()
{
   int i;
   mensaje m;
   
   no_responde = FALSE;
   
   /* No entiendo muy bien por que deben desactivarse las INTs     */
   /* Al limpiar el DOR teoricamente se deshabilita la INT del FDC */
   /* pero aun asi ocurre. Y estropea todo, puesto que desbloquea  */
   /* el proceso bloqueado sin haberse reseteado el controlador.   */
   desactivar();
   nuevo_estado = prendido = FALSE;
   outp8(FD_DOR, 0);
   outp8(FD_DOR, ACTIVAR_INT);    
   activar();

   /* ahora, aceptamos la INT perdida */  
   recibir(HARDWARE, &m);

   for(i = 0; i < 4; i++) {
      out_fd(FD_SENSE);     
      estados_fd();
   }

   calibrado = FALSE;
}


/* ====================================================================== *
 == Espera la INT que el FDC emite al finalizar una operacion.           ==
 == IMPORTANTE: Espera solo un tiempo prudente, si despues de este tiempo==
 == el controlador se niega a responder se resetea el FDC                ==
 ======================================================================== */
#define OCUPADO_ES       0
#define OCUPADO_TIEMPO   1
#define DESOCUPADO       2

PRIVADA int esperar_int()
{
   mensaje m;

   ocupado = OCUPADO_ES;

   /* Le pide al reloj que despues de trancurridos TIEMPO_ESPERA ticks */
   /* ejecute la funcion "tiempo_cumpido" que pone fin a la espera     */
   com_reloj(TIEMPO_ESPERA, tiempo_cumplido);
   
   /* se bloquea a la espera de floppy_irq como de tiempo_cumplido */   
   recibir(HARDWARE, &m);

   /* si se cumpilo el tiempo, se debe resetear el controlador */
   if(ocupado == OCUPADO_TIEMPO) {
      resetear_fd();
      return E_TIMEOUT;
   }

   ocupado = DESOCUPADO;
   return OK;
}



/* ====================================================================== *
 == Despues de TIEMPO_ESPERA ticks el reloj ejecuta esta funcion         ==
 ======================================================================== */
PRIVADA void tiempo_cumplido()
{
   if (ocupado != OCUPADO_ES) return;
   ocupado = OCUPADO_TIEMPO;
   
   interrupcion(T_FLOPPY);
}



/* ====================================================================== *
 == Despues de una operacion, el FDC genera una INT avisando sobre la    ==
 == finalizacion de esta. Entonces debemos leer el estado del FDC o el   ==
 == resultado de la operacion.                                           ==
 ======================================================================== */
PRIVADA int estados_fd()
{
   int i, resultados;
   unt8 estado;
   
   resultados = 0;
   
   for(i=0; i<100000; i++) {
      estado = inp8(FD_MSR) & (STATUS_READY | STATUS_DIR | STATUS_BUSY);
      if (estado == (STATUS_READY | STATUS_DIR | STATUS_BUSY))
      {
         /* como mucho el FDC puede retornar 7 (MAX_ESTADOS)
          * resultados (READ_ID - FORMAT_A_TRACK - VERIFY -
          * READ_A_TRACK - WRITE_DELETEd_DATA - WRITE_DATA -
          * READ_DELETEd_DATA - READ_DATA )
          * Solo DUMPREG retorna 10 resultados pero no usamos
          * este comando.
          */
         if (resultados >= MAX_ESTADOS)  break;
         estados[resultados++] = inp8(FD_DATA);
         continue;
      }
   
      /* cuando el FDC retorna READY (DRQ) es porque no hay
       * mas resultados y esta en condiciones de acptar otro
       * comando.
       */
      if (estado == STATUS_READY) {
         activar_irq(IRQ_FLOPPY);
         return OK;     
      }
   }
   
   /* si estamos aqui es porque hay muchos resultados */
   no_responde = TRUE;
   activar_irq(IRQ_FLOPPY);
   return E_ESTADO;
}


/* ====================================================================== *
 == Coloca el cabezal en la pista correcta.                              ==
 ======================================================================== */
PRIVADA int seek_fd()
{
   int r;

   /* estamos en el cilindo que queremos? */
   if (!calibrado)
      if(recalibrar() != OK) return E_SEEK;

   if (cil_act == cil_real) return OK;
   
   /* Busco el cilindro. Cuando este posicionado el FDC nos avisa con   */
   /* una INT. por lo que enviamos los comandos y esperamos al FDC      */
   out_fd(FD_SEEK);
   out_fd(cabeza << 2);
   out_fd(cil_real);

   if (no_responde) return E_SEEK;
   if (esperar_int() != OK) return E_TIMEOUT;
   
   /* Si llegamos aca es porque recibimos la INT. Asi que leemos el estado*/
   out_fd(FD_SENSE);
   r = estados_fd();
   if ( (r != OK) ||(ST0 & ST0_BITS) != SEEK_ST0  || ST1 != cil_real)
      return E_SEEK;
   

   cil_act = cil_real;
   return OK;
}


PRIVADA dispositivo *preparar(int d)
{
   return &geometria;
}





#define DMA_ADDR       0x004  /* port for low 16 bits of DMA address */
#define DMA_TOP        0x081  /* port for top 4 bits of 20-bit DMA addr */
#define DMA_COUNT      0x005  /* port for DMA count (count =  bytes - 1) */
#define DMA_FLIPFLOP   0x00C  /* DMA byte pointer flip-flop */
#define DMA_MODE       0x00B  /* DMA mode port */
#define DMA_INIT       0x00A  /* DMA init port */
#define DMA_RESET_VAL  0x06


PRIVADA int operar(pid proc, operacion_es *es)
{
   bool  escribo;
   int   bloque;
   dir32 buffer;
   int r, cnt, b, cnt_b;
   
   bloque   = es->pos;
   buffer   = es->buf;
   cnt      = es->cnt;
   
   cnt_b    = cnt/512;
   cnt_b   += cnt%512 ? 1 : 0;
   
   
   cnt--;
   
   sector   = 1 + (bloque % NRO_SECTORES_PISTA);
   cabeza   = (bloque / NRO_SECTORES_PISTA) % NRO_CABEZAS;
   cilindro = bloque / (NRO_SECTORES_PISTA * NRO_CABEZAS);
   escribo  = es->op == ESCRIBIR;
   
   for(b=0; b<cnt_b; b++){
      desactivar();
      outp8(DMA_INIT, DMA_RESET_VAL);    /* reset the dma controller */
      outp8(DMA_FLIPFLOP, 0);            /* write anything to reset it */
      outp8(DMA_MODE, escribo ? DMA_WRITE : DMA_READ);
      outp8(DMA_ADDR, (int) buffer >>  0);
      outp8(DMA_ADDR, (int) buffer >>  8);
      outp8(DMA_TOP, (int) (buffer >> 16));
      outp8(DMA_COUNT, cnt >> 0);
      outp8(DMA_COUNT, cnt >> 8);
      outp8(DMA_INIT, 2);        /* some sort of enable */
      activar();
   
      if((r = wr_fd(escribo))!= OK) break;
      buffer +=512;
   }

   return r;
}


PRIVADA int wr_fd(bool escribo)
{
   int intentos;
   int r;
 
   for(intentos = 0; intentos < NRO_INTENTOS; intentos++)
   {
      r = OK;
      if ((r=seek_fd()) != OK)  continue;

      out_fd (escribo ? FD_WRITE : FD_READ);
      out_fd (cabeza <<2); 
      out_fd (cilindro);
      out_fd (cabeza);
      out_fd (sector);
      out_fd (S512);
      out_fd (NRO_SECTORES_PISTA); 
      out_fd (GAP);
      out_fd (DTL);

      if ((r=esperar_int()) != OK) return E_TIMEOUT;

      /* La calibracion se realizo con exito */
      if((r = estados_fd())==OK) break; 
   
   } /* endfor */

   return r;
}


PRIVADA int finalizar()
{
   apagar_motor();
   return OK;
}




/* ====================================================================== *
 == Le pide al reloj que despues de trancurridos ticks ticks ejecute la  ==
 == funcion f                                                            ==
 ======================================================================== */
PRIVADA void com_reloj(int ticks, void (*f)())
{
   mensaje m;
   
   m.SERVICIO = EST_ALARMA;
   m.TICKS    = ticks;
   m.FUNC     = (dir32)f;
   
   envrec(T_RELOJ, &m);   /* envia la peticion y recibe la respuesta*/
}


PRIVADA void geometria_floppy(particion * entry)
{
   entry->cils = NRO_CILINDROS;
   entry->cabs = NRO_CABEZAS;
   entry->secs = NRO_SECTORES;
}


