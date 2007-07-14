/*********************************************************************************
 *  mfs.c - Manager File System v.1.0.0                                          *
 *  Opera sobre el sistema de archivo SAA en un archivo una imagen               *
 *                                                                               *
 *  mfs disp                                                                     *
 *********************************************************************************/
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "sa.h"


#define NRO_NODOSI	64

typedef enum {false = 0, true } bool ;

FILE * abrir(char *arch, char *modo);
void ayuda();
bool geom_valida();
void formatear();
void bootear();
void est_bit(unsigned long b);
unsigned long obt_bit();
void copiar();
void entrada(int argc, char *argv[]);
void calc_p();

char *booteador;
char *file;

FILE * fdisp;
FILE * ff;

bool p_formatear = false;
bool p_bootear = false;
bool p_copiar = false;

typedef struct {
   int  cabezas;
   int  pistas;
   int  sppista;
   int  spbloque;
   int  bpsect;
   long tot_sects;
   long tot_bloques;
   int  bpbloque;
   long pos_mb;
   int  b_mb;
   long off_mb;
   long pos_mb_ni;
   int  b_mb_ni;
   long off_mb_ni;
   long pos_nodosi;
   int  b_nodosi;
   long off_nodosi;
   long pos_dir;
   int  b_dir;
   long off_dir;
} layout;

layout p;


#define word unsigned short
unsigned char *b1;
unsigned char *b2;



void main(int argc, char *argv[])
{
  printf("mfs v.1.0.0\n");

  if(argc<2) ayuda();

  p.cabezas = 2;
  p.pistas  = 80;
  p.sppista = 18;
  p.spbloque= 2;
  p.bpsect  = 512;

  entrada(argc, argv);
  calc_p();

  fdisp = abrir(&argv[1][0], "rb+");		/* dispositivo */
  if(p_formatear) formatear();
  if(p_bootear)   bootear();
  if(p_copiar)		copiar();
  fcloseall();
}




FILE * abrir(char *arch, char *modo)
{
   FILE *f;
   f = fopen(arch, modo);
   if(f==NULL){
      printf("No se pudo abrir <%s>", arch);
      exit(1);
   }

   return f;
}


void ayuda(){
   printf("mfs.c - Manager File System v.1.0.0\n");
   printf("\tOpera sobre el sistema de archivo SAA en un archivo imagen\n");
   printf("\tmfs disp opciones\n");
   printf("\tdisp\t nombre del arch. imagen.\n");
   printf("\t-f\t formatear el disp. (vea: -c -p -s)\n");
   printf("\t-c\t nro. de cabezas. def:  2.\n");
   printf("\t-p\t nro. de pistas.  def: 80\n");
   printf("\t-s\t nro. de sectores. por pista. def: 18\n");
   printf("\t-b\t instala el booteador\n");
   printf("\t-C\t copia un archivo\n");
   printf("\t-h\t muestra esta ayuda");
   exit(0);
}

bool geom_valida()
{
   if(p.cabezas <1 || p.pistas < 1 || p.sppista <1) return false;
   return true;
}

void formatear()
{
   int i;
   superb *sb;
   nodoi ni[4];
	dir_item * e;

   if(!geom_valida()){
      printf("\nGeometria del disp. no valida.\n");
      printf("-c%d -p%d -s%d\n\n", p.cabezas, p.pistas, p.sppista);
	   ayuda();
   }

   /* Super Bloque */
   memset(b1, 0xff, p.bpbloque);

   for(i=0; i<p.tot_bloques; i++)
     fwrite(b1, p.bpbloque, 1, fdisp);

   fseek(fdisp, 0, SEEK_SET);
   memset(b1, 0x00, p.bpbloque);
   sb = (superb * )b1;

   sb->id[0] = 'S'; sb->id[1] = 'A'; sb->id[2] = 'A';
   sb->ver[0]= '1'; sb->ver[1]= '0'; sb->ver[2]= '0';
   sb->nro_nodosi    = 64;
   sb->nro_cabezas   = p.cabezas;
   sb->nro_pistas    = p.pistas;
   sb->sects_ppista  = p.sppista;
   sb->sects_pbloque = p.spbloque;
   sb->bytes_psector = p.bpsect;
   sb->dispositivo   = 1;
   fwrite(b1, p.bpbloque, 1, fdisp);


   /* Mapa de Bits */
   memset(b1, 0x00, p.bpbloque);
   fseek(fdisp, p.off_mb, SEEK_SET);
   b1[0] = 0xff;  b1[1] = 0xf0;
   fwrite(b1, p.bpbloque, 1, fdisp);
   b1[0] = 0x00;  b1[1] = 0x00;

   for(i=1; i<p.b_mb; i++)
      fwrite(b1, p.bpbloque, 1, fdisp);


   /* Mapa de bits de los nodos I */
   fseek(fdisp, p.off_mb_ni, SEEK_SET);
   b1[0] = 0xf0;
   fwrite(b1, p.bpbloque, 1, fdisp);
   b1[0] = 0x00;

   for(i=1; i<p.b_mb_ni; i++)
      fwrite(b1, p.bpbloque, 1, fdisp);


   /* Nodos I */
   memset(&ni[0], 0, 4 * sizeof(nodoi) );
	/* booteo */
   ni[0].atrib  = ATTR_SISTEMA|ATTR_SOLO_LECTURA|ATTR_ARCHIVO;
   ni[0].disp   = 1;
   ni[0].tam[1] = 1024;
   ni[0].enlaces= 1;
   ni[0].blq[0] = 0;

	/* mapa bits */
   ni[1].atrib  = ATTR_SISTEMA|ATTR_SOLO_LECTURA|ATTR_ARCHIVO;
   ni[1].disp   = 1;
   ni[1].tam[1] = p.b_mb * p.bpbloque;
   ni[1].enlaces= 1;
   for(i=0; i<p.b_mb; i++)
      ni[1].blq[i] = p.pos_mb + i;

	/* nodosi */
   ni[2].atrib  = ATTR_SISTEMA|ATTR_SOLO_LECTURA|ATTR_ARCHIVO;
   ni[2].disp   = 1;
   ni[2].tam[1] = NRO_NODOSI * sizeof(nodoi);
   ni[2].enlaces= 1;
   for(i=0; i<p.b_nodosi; i++)
      ni[2].blq[i] = p.pos_nodosi + i;

   ni[3].atrib  = ATTR_SISTEMA | ATTR_DIRECTORIO;
   ni[3].disp   = 1;
   ni[3].tam[1] = 1024;
   ni[3].enlaces= 1;
   ni[3].blq[0] = p.pos_dir;


   fwrite(ni, 4 * sizeof(nodoi), 1, fdisp);
   memset(b1, 0x00, p.bpbloque);
   fwrite(b1, p.bpbloque-(4 * sizeof(nodoi)), 1, fdisp);

   for(i=1; i<p.b_nodosi; i++)
   	fwrite(b1, p.bpbloque, 1, fdisp);


   /* Dir raiz */
	e = (dir_item *)b1;
   strcpy(e[0].nombre, "booteo");
	e[0].nodo_i = 0;
   strcpy(e[1].nombre, "mb");
	e[1].nodo_i = 1;
   strcpy(e[2].nombre, "nodosi");
	e[2].nodo_i = 2;
   strcpy(e[3].nombre, "\\");
	e[3].nodo_i = 3;
   fseek(fdisp, p.off_dir, SEEK_SET);
   fwrite(b1, p.bpbloque, 1, fdisp);
}


void bootear()
{
   ff = abrir(booteador, "rb");

   fread(b1, p.bpbloque, 1, ff);

   fseek(fdisp, 0, SEEK_SET);
   fread(b2, p.bpbloque, 1, fdisp);
   fseek(fdisp, 0, SEEK_SET);

   memcpy(&b1[3], &b2[3], sizeof(superb)-3);
   fwrite(b1, p.bpbloque, 1, fdisp);

   fclose(ff);
}


void est_bit(unsigned long b)
{
   unsigned long i, bit;
   unsigned char mascara;

   i   = b/8;
   bit = 7 - b%8;
   mascara = 1<<bit;

   b1[i] |= mascara;
}


unsigned long obt_bit(long off, long tot_bytes){
   int b, i, j;

   fseek(fdisp, off, SEEK_SET);
   fread(b1, p.bpbloque, 1, fdisp);

   for(i=0; i < tot_bytes; i++ )
      if(b1[i]<0xff)
         for(j=0; j< 8; j++)
            if(!(b1[i] & (0x80 >> j)))
            {
               b = (i*8) + j;
               est_bit(b);
               fseek(fdisp, off, SEEK_SET);
               fwrite(b1, tot_bytes, 1, fdisp);
               return b;
            }

   return 0;
}



void copiar()
{
   long len, posi, posd;
   int blqs, blqs_mb, blq_nodoi;
   int i, j;
   unsigned long bl;
   bool simple;
   nodoi ni;
   dir_item e;
   unsigned long * ptr;

   simple = false;

   ff = abrir(file, "rb");

  /* obtenemos el tamaño del archivo */
   fseek(ff, 0, SEEK_END);
   len = ftell(ff);
   fseek(ff, 0, SEEK_SET);

  /* calculamos la cantidad de bloques que ocupara */
   blqs = (len / p.bpbloque) + 1;

  /* obtener nodoi (libre) */
   i = obt_bit(p.off_mb_ni, NRO_NODOSI / 8);
   if(i==0){
     printf("\nno hay nodosi disponibles!!.");
     exit(1);
   }

  /* calculamos la posicion del nodo i*/
   posi = p.off_nodosi + i * sizeof(nodoi);

  /* obtengo una entrada de directorio */
   fseek(fdisp, p.off_dir, SEEK_SET);     /* vamos al directorio */
   fread(&e, sizeof(e), 1, fdisp);        /* leemos las entradas de a una buscando una con */
   while(e.nombre[0] !=0 )                /* el primer caracter del nombre igual a 0 */
      fread(&e, sizeof(e), 1, fdisp);     /* aca hay un error porque supone que siempre hay */
                                          /* una entrada libre. */

  /* la possicion de la entrada en el directorio esta dada por
   * la posicion actual menos la longitud de la entrada recien
   * leida.
   */
   posd = ftell(fdisp)- sizeof(e);

   /* pongo los atributos */
   memset(&ni, 0, sizeof(nodoi));
   ni.atrib    = ATTR_ARCHIVO;
   ni.disp     = 1;
   ni.tam[1]   = len;
   ni.enlaces  = 1;

   strcpy(&e.nombre[0], file);
   e.nodo_i    = i;

   /* Ahora por cada bloque */
   for(j = 0; j< blqs; j++)
   {
	/* obtengo un bloque libre */
      bl = obt_bit(p.off_mb, p.tot_bloques/8);

      /* si el bloque es menor que 23 se encuentra en el
       * directo.
       */
      if(j<23)
         ni.blq[j] = bl;

      /* sino, puede que se encuentre en el bloque simple
       * indirecto. por ejemplo 
      else if( j < ((p.bpbloque/4)+ 23 ))		/* 8 bytes del ptr. a bloque */
      {
         /* si es el bloque 24 */
         if(!simple){
            ni.blqS = bl;                       /* asigno este bloque para que sea el simple */
            bl = obt_bit(p.off_mb, p.tot_bloques/8); /* indirecto y pido otro para guardarlo */
            simple = true;
         }

         /* vamos al bloque virtual nro. ni.blqS, leemos el bloque correspondiente
          * al nuevo bloque simple indirecto y registramos el nuevo bloque.
          */
         fseek(fdisp, ni.blqS * p.bpbloque, SEEK_SET);
         fread(b1, p.bpbloque, 1, fdisp);
         ptr = b1;
         ptr[j-23] = bl;
         fseek(fdisp, ni.blqS * p.bpbloque, SEEK_SET);
         fwrite(b1, p.bpbloque, 1, fdisp);
      }

      /* Limpiamos el buffer con ceros, leemos el bloque del archivo
       * y lo guardamos.
       */
      memset(b2, 0x00, p.bpbloque);
      fread(b2, p.bpbloque, 1, ff);
      fseek(fdisp, bl * p.bpbloque, SEEK_SET);
      fwrite(b2, p.bpbloque, 1, fdisp);
   }

   /* guardo el nodo i */
   fseek(fdisp, posi, SEEK_SET);
   fwrite(&ni, sizeof(ni), 1, fdisp);

   /* guardo el dir */
   fseek(fdisp, posd, SEEK_SET);
   fwrite(&e, sizeof(e), 1, fdisp);

   fclose(ff);
}



void entrada(int argc, char *argv[])
{
  int i;
  for(i=2; i<argc; i++){
    if(argv[i][0] != '-') ayuda();
    switch(argv[i][1]){
      case 'h':  ayuda();
      case 'f':  p_formatear = true;  break;
      case 'b':  p_bootear = true; booteador = &argv[i][2]; break;
      case 'c':  p.cabezas = atoi(&argv[i][2]); break;
      case 'p':  p.pistas  = atoi(&argv[i][2]); break;
      case 's':  p.sppista = atoi(&argv[i][2]); break;
      case 'C':  p_copiar = true; file = &argv[i][2]; break;
      default : ayuda();
   }
  }
}


void calc_p()
{
   p.tot_sects = p.cabezas * p.pistas * p.sppista;
   p.tot_bloques = p.tot_sects / p.spbloque;
   p.bpbloque  = p.bpsect * p.spbloque;

   p.pos_mb    = 1;
   p.b_mb      =  (p.tot_bloques / 8) / p.bpbloque;
   p.b_mb     += ((p.tot_bloques / 8) % p.bpbloque) > 0 ? 1 : 0;
   p.off_mb    = p.pos_mb * p.bpbloque;

   p.pos_mb_ni = p.pos_mb + p.b_mb;
   p.b_mb_ni   =  (NRO_NODOSI /8 ) / p.bpbloque;
   p.b_mb_ni   = ((NRO_NODOSI /8 ) % p.bpbloque) > 0 ? 1 : 0;
   p.off_mb_ni = p.pos_mb_ni * p.bpbloque;

   p.pos_nodosi= p.pos_mb_ni + p.b_mb;
   p.b_nodosi  =  (NRO_NODOSI * sizeof(nodoi)) / p.bpbloque;
   p.b_nodosi += ((NRO_NODOSI * sizeof(nodoi)) % p.bpbloque) > 0 ? 1 : 0;
   p.off_nodosi= p.pos_nodosi * p.bpbloque;

   p.pos_dir   = p.pos_nodosi + p.b_nodosi;
   p.b_dir     = 1;
   p.off_dir   = p.pos_dir * p.bpbloque;

   b1 = (unsigned char *)malloc(p.bpbloque);
   b2 = (unsigned char *)malloc(p.bpbloque);
}


