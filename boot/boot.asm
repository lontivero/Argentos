;/* ===================================================================== *
; ==  Este archivo contiene el codigo para el bloque 0 (sector de booteo)==
; ==  Se llama bloque 0 y no sector de booteo porque este codigo acupa   ==
; ==  mas de 512 bytes por lo que no entra en un solo sector. En un      ==
; ==  intento anterior existian el sector de booteo y el cargador por    ==
; ==  separado (el cargador estaba escrito en C) pero esto es mucho mejor==
; ==  puesto que solo se tienen 2 archivos binarios: boot.bin y kernel   ==
; ==                                                                     ==
; ==  Objetivo:                                                          ==
; ==    + cargar el sistema (o informar al usuario sobre la ausencia)    ==
; ==    + entrar en modo protegido                                       ==
; ==    + saltar al nucleo                                               ==
; =========================================================================
; ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++      ==
; ==    ++ Por: Lucas Ontivero                                           ==
; ==    ++ email: lucasontivero@yahoo.com.ar                             ==
; ======================================================================== */
.model tiny
.code
.386p

entry_point:
   jmp     inicio       ; ocupa 3 bytes
   nop

;=======================================================================
; Registro de booteo                  
;=======================================================================
   id          db 'SAA'    ; nro. magico
   ver         db '100'    ; version 1.0.0
   nro_nodosi  dd 64
   cabezas     db 2
   pistas      dw 80
   sppista     dw 18
   spbloque    db 2   
   bpsector    dw 512
   disp        db 0


;=======================================================================
;  Comienza el código
;=======================================================================
inicio:
   cli                        ; limpio las ints. para cambiar los Seg.
   xor   ax, ax               ; todos a 0
   mov   ds, ax
   mov   es, ax
   mov   ss, ax
   mov   sp, 7c00h
   sti

   ;-------------------------------
   ; leo el segundo sector del dsk
   ; en 7e00h
   mov   ax, 0201h
   mov   bx, 7E00h
   mov   cx, 0002h            ; del sector 02h pista 00h
   xor   dx, dx               ; disco 00h (A:) cabeza 00h
   int   013h                 ; BIOS
   cmp   ah, 00h              ; hubo error?
   jne   error                ;     notificar
   jmp   main                 ; ahora si vamos al main



;=======================================================================
;  void  leer_bloque(int logico, int dir)
;=======================================================================
;  CX = logico
;  BX = dir 
leer_bloque:
   call    progreso           ; .

   push  bx
   
   mov   ax, cx
   mul   spbloque
   mov   cx, ax               ; cx = sect_absoluto

   cwd
   div   sppista
   inc   dx
   push  dx                   ; push sector
         
   mov   ax, cx
   cwd   
   div   sppista
   div   cabezas
   shr   ax, 8
   push  ax                   ; push cabeza
   
   mov   ax, sppista
   mul   cabezas
   mov   bx, ax
   mov   ax, cx
   cwd  
   div   bx                   ; ax = pista
                              
   pop   bx                   ; bx = cabeza
   pop   cx                   ; cx = sector
         
   mov   dx, bx
   shl   dx, 8                ; dx = (cabeza<<8) | drive
         
   push  cx
   mov   cx, ax
   shl   cx, 8
   pop   bx
   or    cl, bl               ; cx= (pista<<8)|sector
   
   pop   bx
        
   mov   ah, 02h
   mov   al, byte ptr spbloque   ; cuantos sectores leer
   int   13h                  ; lee
   cmp   ah, 00h              ; hubo error?
   jne   error                ;     notificar
   
   ret


;=======================================================================
;  no_return error(char nro_error)
;=======================================================================
;  AH = nro_error
error:
   shr   ax, 08h              ; al = nro_error
   call  IntToHex             ; ax = itoa(al)
   mov   word ptr [str_fdd_e], ax
   mov   si, offset str_fdd_err
   call  panico



;=======================================================================
;  bool strcmp(char * s1, char * s2)
;=======================================================================
;  SI = s1
;  DI = s2
;     RETURN AX= true si es igual o false si es distinto
strcmp:
   call     progreso       ; .

   jmp   short @1@4
@1@2:
   mov   al, byte ptr [si]
   mov   ah, byte ptr [di]
   inc   si
   inc   di
   cmp   ah, al
   je    short @1@4
   xor   ax,ax                ; return false
   jmp   short @1@6
@1@4:    
   cmp   byte ptr [si],0
   jne   short @1@2
         
   mov   ax,1                 ; return true
@1@6:
   ret


;=======================================================================
;   void cargar_arch(int nro_nodoi, char far * buf)
;=======================================================================
;   AX = nro_nodoi
;   CX = buf
;
;   funcionamiento:
;      sabiendo que los nodosi se encuentran en 08000h y dado el nro de
;      nodoi.
;          + calcula la cantidad de bloques (blqs) que ocupa el archivo que se
;            debe leer.
;          + realiza un bucle de 0 a blqs-1 donde extrae del nodoi, el nro. de
;            bloque que se debe leer y calcula donde ponerlo.
;              + si ya se leyeron los 23 primeros bloques, extrae del nodoi el
;                nro. de bloque simple indirecto y lo pone en 09000h
;              + calcula el proximo bloque y lo lee.
;
cargar_arch:
    call    progreso

   ;      t = (ni[i].tam[1] / bpb) + 1;
   shl   ax, 7                ; ax = nro. nodo_i * 128 bytes c/u
   add   ax, 08000h           ; ax = ni[i]
   mov   bx, ax               ; bx = ni[i]
   add   ax, 7                ; ax = ni[i].tam[1]
   mov   si, ax
   mov   ax, word ptr [si]
   cwd
   div   bpb                  ; ax = ni[i].tam[1] / bpb
   inc   ax                   ; ax = cant. de bloques a leer
         
   ;      for(j=0; j<t; j++, buf+=bpb)
   xor   dx, dx
   mov   j, dx                ; j = 0
   jmp   short @5@6
@5@3:
   ;        if(j < 23)        ; se leyeron 23 o mas bloques?
   cmp   j, 23                ; porque son 23 ptrs directors en el nodoi
   jae   short @5@8

   ;        leer_bloque(ni[i].blq[j], buf);
   push  bx
   push  ax
   mov   ax, 4                ; 4 bytes porque el ptr es de 32 bits.
   mul   j                    ; ax = blq[j]
   add   ax, 24               ; mas offset de blq[0]
   add   ax, bx               ; ax = ni[i].blq[j]

   push  cx
   push  cx
   mov   bx, ax
   mov   cx, [bx]             ; cx = nro_bloque
   pop   bx                   ; bx = buffer donde leer (cx)
   jmp   @5@9

@5@8:
   push  bx
   push  ax
   push  cx
   push  cx

   ; if(!leido)               ; ya se leyo el bloque simple ind.?
   cmp   byte ptr leido, 00h
   jne   short @5@7

   ;     leer_bloque(ni[j].blqS, 0x9000h);   /* lee el bloque simple ind. */
   add   bx, 116              ; bx = ni[j].blqS;
   mov   cx, word ptr [bx]    ;
   push  es
   push  ds
   pop   es
   mov   bx, 09000h
   call  leer_bloque
   pop   es
   inc   byte ptr leido       ; leido = true;

@5@7:
   mov   bx, j                ; calculo el off. del prox. nro. de blq.
   sub   bx, 23               ; j - 23
   shl   bx, 2                ; off = 4*(j-23)
   add   bx, 09000h           ; off = 0x9000 + 4*(j-23)  / off = bloques[j-23] 
   mov   cx, word ptr [bx]    ; cx = nro_bloque   
   pop   bx

@5@9:
   call  leer_bloque
   pop   cx
   pop   ax
   pop   bx

   inc   j
   add   cx, bpb              ; buf += bpb     
@5@6:
   cmp  word ptr j, ax
   jl   short @5@3
   ret

    

;   IntToChar
;   AX = numero
IntToHex:
   xor   ah, ah
   shl   ax, 4
   shr   al, 4
   xchg  ah, al
   add   ah, '0'
   cmp   ah, '9'
   jna   _1
   add   ah, 8
_1:
   add   al, '0'
   cmp   al, '9'
   jna   _2
   add   al, 8
_2:
    ret


;=======================================================================
;  void progreso(void)
;=======================================================================
;   muestra un punto en la pantalla.
progreso:
   push  ax
   push  bx
   mov   bx, 07h
   mov   ah, 0eh
   mov   al, '.'
   int   10h
   pop   bx
   pop   ax
   ret


;=======================================================================
;  void prints(char *text)
;=======================================================================
;  SI = text
prints:
   mov   bx, 07h
   mov   ah, 0eh
   
   jmp   short @3@3
@3@2:    
   mov   al, byte ptr [si]
   int   10h
   inc   si
@3@3:    
   cmp   byte ptr [si],0
   jne   short @3@2
   ret



;=======================================================================
;  no_return panico(char *text)
;=======================================================================
;  SI = text
panico:
   call  prints
   call  pausa
   db    0EAh                 ; jmp     far 0F000h:0FFF0h
   dd    0F000FFF0h           ; reboot


;=======================================================================
;  void pausa(void)
;=======================================================================
pausa:
   mov   si, offset str_pausa
   call  prints
   xor   ax,ax
   int   16h
   ret


;=======================================================================
;  debemos pones al final del sector la marca 55AA para que el BIOS
;     sepa reconozca que se trata de un sector de booteo valido.
;=======================================================================
   org   01feh
   signature  db 055h, 0AAh


;=======================================================================
;  no_return main(void)
;=======================================================================
;     + calcula el bloque de los nodosi
;     + lee el primer bloque de nodosi (suficiente porque DIR es el nro 3)
;       en 08000h
;     + carga todo el DIR RAIZ en 0x8000 + bytes_per_bloque
;     + busca el archivo 'kernel'
;       + si no lo encuentra 'Disco sin sistema!!!' y fin. :(
;       + si lo encuentra lo carga en 010000h
;     + GO TO Protected Mode (modo magico!)  
;     + JUMP TO my kernel  :)
;
main:
   ; limpia la pantalla
   mov   ax, 0003h
   int   10h

   mov   si, offset str_carga
   call  prints 
   
   ;     dir_item far * e;
   ;     bpb = rec->bytes_psector* rec->sects_pbloque;
   mov   ax, bpsector
   mul   word ptr spbloque
   mov   bpb, ax

   ;     tot_blks = (rec->nro_cabezas * rec->nro_pistas * rec->sects_ppista)/rec->sects_pbloque;
   xor   ax, ax
   mov   al, cabezas
   mul   pistas
   mul   sppista
   div   word ptr spbloque

   ;     blks_mb  = ((tot_blks/8)/ bpb)+1;
   shr   ax, 3
   div   bpb
   inc   ax

   ;     leer_bloque(1+blks_mb, ni);  /* lee nodos i */
   inc   ax
   inc   ax
   mov   cx, ax
   mov   bx, 08000h
   call  leer_bloque

   ;      e = MK_FP(0x800, bpb);
   mov   cx, bpb
   add   cx, 8000h            ; cx = directorio raiz. 
   mov   e,  cx

   ;     cargar_arch(3, e);  /* cargo el dir raiz */
   mov   ax, 3
   call  cargar_arch  

   ;     for(i=0; e->nombre[0] != 0; i++, e++)
   jmp   short @6@5
@6@2:
   ;         if(strcmp(e->nombre, "booteo")){
   mov   si, e
   mov   di, offset str_archivo
   call  strcmp
   or    ax,ax
   je    short @6@4
   ;            existe = true;
    mov  ax, 1
   ;            break;
   jmp   short @6@6
@6@4:
   add   e, 64
@6@5:
   mov   bx, e
   cmp   byte ptr [bx],0
   jne   short @6@2
   xor   ax, ax
@6@6:
   ;     if(!existe)

   cmp   ax, 0
   jne   short @6@8

   ;     panic("\nDisco sin sistema.");
   mov   si, offset str_sistema
   call  panico
@6@8:
   ;      cargar_arch(e->nodo_i, MK_FP(0x1000,0));
   mov   bx, e
   add   bx, 48
   mov   ax, word ptr [bx]
   mov   cx, 1000h            ; @@ cambiar
   mov   es, cx
   xor   cx, cx
   call  cargar_arch


;=======================================================================
; Todos los registros de segmento (salvo CS) iguales a 0 para que sean
; validos cuando pasemos a modo protegido. SP = 0x1000
;=======================================================================
   cli
   xor   ax, ax
   mov   es, ax
   mov   ds, ax
   mov   ss, ax
   mov   sp, 1000h
   mov   gs, ax
   mov   fs, ax
   sti  
        
   jmp   PMODE_Start
;=======================================================================
; Activamos la linea A20
;=======================================================================
ActivarA20:
   call  vacia8042
   mov   al,0D1h     
   out   064h,al
   call  vacia8042
   mov   al,0DFh     
   out   060h,al
   call  vacia8042
   ret

;=======================================================================
; Chequeamos que la cola de comandos del teclado este vacia
;=======================================================================
vacia8042:
@8@1:
   call  espera
   in    al,064h
   test  al,1
   jz @8@2                    ; sin salida

   call  espera
   in    al,060h     
   jmp   @8@1                 ; vacia  

@8@2:
   test  al,2
   jnz   @8@1                 ; vacia
   ret

;=======================================================================
; Una pequena pausa
;=======================================================================
espera:  
   jmp   @9@1
@9@1:
   jmp   @9@2
@9@2:
   ret


;=======================================================================
; Vamos a modo protegido
;=======================================================================
PMODE_Start:

   call  ActivarA20
   
   cli
   db 066h
   lgdt  [fword ptr gdt_48]

   mov   eax, cr0
   or    al, 1
   mov   cr0, eax

   jmp   $+2
   nop

; Saltamos al kernel 0x8:0x10000
   db 67h
   DB 66h
   DB 0EAh                    ; salto 
   DD 010000h                 ; direccion de kernel (_start)
   DW    0008                 ; selector de CODIGO


;=======================================================================
;  VARIABLES
;=======================================================================
   j           dw 0
   e           dw 0
   bpb         dw 0
   leido       db 00h

;=======================================================================
;  CADENAS
;=======================================================================
   str_carga   db  "ARGENT-OS ver 1.0.0a Booteando", 0
   str_prog    db  '.', 0
   str_fdd_err db  0Ah, 0Dh, "Error de disco "
   str_fdd_e   db  0, 0, 'h', 0Ah, 0Dh, 0
   str_pausa   db  "Presione una tecla para rebootear...", 0Ah, 0Dh, 0
   str_sistema db  "Disco sin sistema.", 0Ah, 0Dh, 0
   str_archivo db  "kernel", 0


;=======================================================================
;  El pseudo descriptor para la gdt
;=======================================================================
gdt_48:
   dw 10*8-1                  ; Limite
   dd BIG_GDT                 ; dir.


;=======================================================================
;  GDT
;=======================================================================
BIG_GDT:
   dw 0,0,0,0

; CODIGO indice 1 0x8
   ; 4Gb - base 0 - segm. de codigo - ganular
   dw 0FFFFh, 00000h, 09A00h, 000CFh

; DATOS  indice 2 0x10
   ; 4Gb - base 0 - segm. de datos (wr) - ganular
   dw 0FFFFh, 00000h, 09200h, 000CFh

; Los demas no son validos
;  db 256 dup(0)


end entry_point
end

   