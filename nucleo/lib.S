/*
 * lib.S
 */

.text

/**************************************************************
 * _cp_mem : Copia bloques de memoria.                         
 *************************************************************/
  .global _cp_memx
   .align   4
_cp_memx:
   push  %ebp
   mov   %esp,%ebp
   push  %ecx
   mov   0x08(%ebp),%edi     /* destino  */
   mov   0x0c(%ebp),%esi     /* origen   */
   mov   0x10(%ebp),%ecx     /* cantidad */
   cmp   %edi,%esi
   je    fin_cp_mem          /* si(origen == destino) return; */

   cmp   %esi,%edi
   jb    de_izq_der          /* si(destino < origen) copiar_derecho */
   lea   (%ecx,%esi,1), %eax /* eax = origen + cantidad */
   cmp   %eax,%edi
   jae   de_izq_der          /* si(destino >= origen + cantidad) copiar_derecho */

   lea   (%ecx,%edi,1), %edi
   lea   (%ecx,%esi,1), %esi
   dec   %edi
   dec   %esi
   dec   %ecx
   std
   jmp    de_der_izq

de_izq_der:
   cld
de_der_izq:
   cmpl  $4, %ecx    /* si son menos de 4 bytes */
   jb de_a_uno       /* copio de a uno */
   shrl  $2, %ecx    /* Sino copio de a 4 porque es mas rapido */
   rep               /* Repetir */
   movsl             /* Mover de a 4 */
   movl  0x10(%esp), %ecx  /* ecx = cantidad */
   andl  $3, %ecx    /* ecx = (cantidad % 4) */
   jnz   de_a_uno
   jmp   fin_cp_mem  /* terminado */

de_a_uno:
   rep               /* Repetir */
   movsb             /* Mover de a 1 */

fin_cp_mem:
   pop   %ecx
   pop   %ebp
   ret   
