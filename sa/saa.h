/*#include "tipos.h"*/

/*
   La cant. de bloques usados para el mapa de bits esta en relacion con el
   tamano del dispositivo.  Asi, un disquette de  1,44 MB (2880 bloques de
   512 bytes) tendra 2880 bits (360 bytes) en su mapa. Esto es 1 bloque.
   
   cnt. blqs. = (tam. disp.)/(tam. bloque)
   cnt. blqs. mapa de bits = (cnt. blqs./8)/(tam. blq)

   cnt. blqs. nodos i = ((cant. nodos i)*(tam. nodos i))/(tam. blq)

   primer blq. de datos = 2 + (cnt. blqs. mapa de bits) + (cnt. blqs. nodos i)

   bloques		pos	tam (en blqs)
   -------        ---	---
   b. booteo	0	1
   b. super		1	1
   b. mapa bits	2	x1
   b. nodos i	x1	x2
   b. de datos	x2	xf

   los nodos i
   -----------
   i(0) prg de arranque.
   i(1) blqs defectuosos.
   i(2) mapa de bits.
   i(3) dir. raiz.

   los atributos
   -------------
   del sistema		00000001	1
   solo lectura		00000010	2
   dispositivo		00000100	4
   directorio		00001000	8
   de archivo		00010000	10
   ejecutable		00100000	20
*/

/* ====================================================================== *
 == tipos                								 ==
 ======================================================================== */
typedef struct {
  char   magico[3];		/* nro. magico 'SB\0'	*/
  unt32  cnt_nodos_i;
  unt16  cabezas, pistas, sectores;
  unt16  bytes_por_sector;
  unt8   sectores_por_bloque;
  unt8   drive;
} superbloque;



typedef struct {
    unt8 jmp[3];
    unt8 oemstr[8];
    unt16 bytes_psector;
    unt8 sec_pcluster;
    unt16 sec_reserv;
    unt8 fats;
    unt16 entries_proot;
    unt16 tot_sects;
    unt8 media_descrip;
    unt16 sec_pfat;
    unt16 sec_ppista;
    unt16 cabezas;
    unt16 sec_ocultos;
    unt32 tot_sectsx;
    unt8 drive;
    unt8 reserv1;
    unt8 signature;
    unt32 serial_numbre;
    unt8 volumen[11];
    unt8 reserv2[8];
} boot_sector;



typedef struct {
    unt8  atrib;
    unt8  disp;
    unt32 tam;
    unt16 enlaces;
    int8  fh_crea[6];
    int8  fh_modi[6];
    int8  fh_acce[6];
    unt16 bloques[16];
    unt16 bloque_s;
    unt16 bloque_d;
    unt16 bloque_t;
} nodo_i;

/* Atributos */
#define ATTR_SISTEMA		0x01
#define ATTR_SOLO_LECTURA	0x02
#define ATTR_DISPOSITIVO	0x04
#define ATTR_DIRECTORIO		0x08
#define ATTR_ARCHIVO		0x10
#define ATTR_EJECUTABLE		0x20

/* Nodos i predeterminados */
#define NODOI_BOOTEO		0
#define NODOI_DEFECTUOSOS	1
#define NODOI_MAPA_BITS		2
#define NODOI_RAIZ		3


typedef struct {
    unt8  nombre[62];
    unt16 nodoi;
} entrada_dir;




