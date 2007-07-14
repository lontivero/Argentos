/* ====================================================================== *
 ==  Este archivo contiene las constantes, estructuras, macros y         ==
 ==  prototipos de funciones necesarias para el manejo del servidor de   ==
 ==  archivos.										 ==
 ==========================================================================
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++	 ==
 ==    ++ Por: Lucas Ontivero								 ==
 ==    ++ email: lucasontivero@yahoo.com.ar					 ==
 ======================================================================== */
#ifndef _MSO_SA_H_
#define _MSO_SA_H_

#include <const.h>

typedef struct {
	unsigned char jmp[3];          /* 3		*/
	unsigned char id[3];           /* 3		*/
	unsigned char ver[3];          /* 3		*/

	unsigned long  nro_nodosi;     /* 4		*/
	unsigned char  nro_cabezas;    /* 1		*/
	unsigned short nro_pistas;     /* 2		*/
	unsigned short sects_ppista;   /* 2		*/
	unsigned char  sects_pbloque;  /* 1		*/
	unsigned short bytes_psector;  /* 2		*/
	unsigned char  dispositivo;    /* 1		*/
} superb;


/*     31   27|26  23|22      17|16   12|11       6|5       0
 *	+-------+------+----------+-------+----------+---------+
 *	|  dias | meses| annos    | hrs.  | mins.    | segs.   |
 *	+-------+------+----------+-------+----------+---------+
 */

typedef unsigned long	f_y_h;
#define SEG_OFF	0
#define MIN_OFF	6
#define HRS_OFF	12
#define ANO_OFF	17
#define MES_OFF	23
#define DIA_OFF	27

#define SEG_MSK	0x0000003F
#define MIN_MSK	0x00000FC0
#define HRS_MSK	0x0000F800
#define ANO_MSK	0x007E0000
#define MES_MSK	0x78000000
#define DIA_MSK	0xF8000000


/* 128 bytes */
typedef struct {
	unsigned char  atrib;      /* 1		*/
	unsigned short disp;       /* 2		*/
	unsigned long  tam[2];     /* 8		*/
	unsigned char  enlaces;    /* 1		*/
	f_y_h creacion;            /* 4		*/
	f_y_h modificacion;        /* 4		*/
	f_y_h acceso;              /* 4		*/
                                 /* 24		*/

	unsigned long blq[23];     /* 4 * 23 = 92 */
	unsigned long blqS;        /* 4		*/
	unsigned long blqD;        /* 4		*/
	unsigned long blqT;        /* 4		*/
} nodoi;


typedef struct {
	unsigned long	primero;
	unsigned long	libre;
} dir;

#define MAX_NOMBRE	48
typedef struct {
	char	nombre[MAX_NOMBRE];
	unsigned long nodo_i;
	unsigned long izq;
	unsigned long der;
	unsigned long hash;
} dir_item;

/* Atributos	*/
#define ATTR_SISTEMA		   0x01
#define ATTR_SOLO_LECTURA	0x02
#define ATTR_DISPOSITIVO	0x04
#define ATTR_DIRECTORIO		0x08
#define ATTR_ARCHIVO		   0x10
#define ATTR_EJECUTABLE		0x20

/* Nodos i predeterminados */
#define NODOI_BOOTEO		0
#define NODOI_DEFECTUOSOS	1
#define NODOI_MAPA_BITS		2
#define NODOI_NODOI		3
#define NODOI_RAIZ		4


PUBLICA void t_sa();

#endif

