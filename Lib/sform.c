#include <sform.h>
 
#define es_digito(a)    (a >= '0' && a<= '9')
#define es_acentuada(a) (a == 'á' || a== 'é' || a == 'í' || a== 'ó' || a == 'ú')
#define es_hispana(a)   (es_acentuada(a) || a == 'ñ' || a== 'Ñ' || a == 'ü')
#define es_alfa(a)      ((a >= 'A' && a<= 'Z') || (a >= 'a' && a<= 'z') || es_hispana(a))

#define CEROS	1

int n(char **num){
  int i;
  i = 0;
  while(es_digito(**num)){    
	i = i*10 + *((*num)++) - '0';
  }
  return i;
}

int dividir(int *n, int base){
  int ret;
  ret = *n % base;
  *n /= base;
  return ret;
}


int numero(char *buff, int num, int base, int ancho, char tipo){
  char c, tmp[36];
  char *digitos="0123456789ABCDEF";
  int i = 0;
  int j = 0;
  
  c= (tipo & CEROS) ? '0' : ' ';

  if(base==10 && num<0){
     buff[i++] = '-';
     num *= -1;
  }

  if(num==0){ buff[i++]='0'; ancho--; }
  else while (num!=0)
     tmp[j++]=digitos[dividir(&num, base)];

  if(ancho>j)
     while(ancho-->j) buff[i++]= c;

  j--;
  for(; j>=0; j--)
     buff[i++]=tmp[j];

  return i;
}

char * formatear(char * buff, char * f, l_arg argumentos)
{
  char * b = buff;
  char * t;  
  int  ancho;
  char tipo = 0;

  *b = *buff = 0;

  while(*f){
    if(*f!='%'){ *b++ = *f++;   continue; }

    f++;
    if(*f=='0' && *(f+1)=='x'){ *b++ = '0'; *b++ = 'x'; f+=2; }
    if(*f=='0' && *(f+1)=='X'){ *b++ = '0'; *b++ = 'X'; f+=2; }

    ancho = -1;
    
    if(es_digito(*f)){
       if(*f=='0') tipo |= CEROS; 
       ancho = n(&f); 
    }

    switch(*f){
       case 'd':
       case 'i': b += numero(b, arg(argumentos, int), 10, ancho, tipo); break;
       case 'x':
       case 'X': b += numero(b, arg(argumentos, int), 16, ancho, tipo); break;

       case 'b': b += numero(b, arg(argumentos, int),  2, ancho, tipo); break;
       case 'f': break;
       case 'c': *b++ = arg(argumentos, int); break;
       case 's': 
                 t = arg(argumentos, char*);
                 while(*t) *b++ = *t++;
                 break;
       case '%': *b++ = '%'; break;
    }
    f++;
  }
  *b=0;
  return buff;
}

