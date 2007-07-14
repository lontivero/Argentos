/*
*  dsk2img.c
*  crea un archivo imagen de un disquette.
*
*  dsk2img -u[unidad] -o[salida] -s[sectores]
*/
#include <dos.h>
#include <stdio.h>
#include <conio.h>

char *archivo;
char unidad   = 0;
int  sectores = 2880;

#define word	unsigned short
unsigned char buffer[0x200];

void cargar_sector(word logico)
{
    word sector = 1 + (logico % 18);
    word cabeza = (logico / 18) % 2;
    word pista  =  logico / (18 * 2);

    word dx= (cabeza<<8);
    word cx= (pista<<8) | sector;

   _BX = (word)buffer;
   _CX = cx;
   _DX = dx;
   _ES = _DS;
   _AX = 0x0201;
   __int__(0x13);
}

void ayuda(){
   printf("dsk2img  crea un archivo imagen de una unidad de disco flexible.\n");
   printf("\tdsk2img -h -u[unidad] -o[salida] -s[sectores]\n");
   printf("\t\t-u\t unidad de disco flexible\n");
   printf("\t\t-o\t nombre del archivo imagen\n");
   printf("\t\t-s\t cant. de sectores a considerar\n");
   printf("\t\t-h\t muestra esta ayuda");
   exit(0); 
}


void entrada(int i, char *argv[])
{
   if(argv[i][0] != '-') ayuda();
   switch(argv[i][1]){
     case 'h': ayuda(); 
     case 'u': unidad  = argv[i][2] == '0' ? 0 : 1; break;
     case 'o': archivo = &argv[i][2]; break; 
     case 's': sectores = atoi(&argv[i][2], 10); break;
   }
}

int main(int argc, char *argv[]){
  word i;
  int j; 
  char *arch = "a.img";
  FILE *img;

  printf("dsk2img v.0.0.1a\n");
  
  archivo = arch; 

  if(argc>4) ayuda();
  for(j=1; j<argc; j++) entrada(j, argv);

  if(sectores>2880)
  {
      printf("sectores > 2880\n");
      exit(0);
  }

  img = fopen(archivo, "wb");
  if(img==NULL)
  {
      printf("No se pudo crear %s\n", archivo);
      exit(0);
  }

  

  for(i=0; i<sectores; i++){
    cargar_sector(i);
    fwrite(buffer, 0x200, 1, img);
  }
 
  for(i=0; i<2880-sectores; i++)
    fwrite(buffer, 0x200, 1, img);

  fclose(img);
}

