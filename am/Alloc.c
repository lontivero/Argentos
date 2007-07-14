
#define NR_ESPACIOS            128    /* cant. de entradas en la tabla de espacios */

PRIVADA typedef struct {
   dir32  direccion;                   /* dereccion del inicio del espacio */
   unt332 capacidad;                   /* cuan grande es en bytes */
   struct espacio *prox;               /* puntero a la proxima entrada en la lista*/
   struct espacio *ant;                /* puntero a la entrada anterior en la lista*/
} espacio;

PRIVADA struct espacios[NR_ESPACIOS];  /* tabla de es pacios   */
PRIVADA struct espacio *cabeza;        /* puntero al primer espacio de la lista */
PRIVADA struct espacio *libre;         /* puntero a la lista de espacios libres */


/*============================================================================
 == tomar_espacio                                                           ==
 == Busca un espacio de memoria libre lo suficientemente grande como para   ==
 == alojar la cantidad de byte solicitados en el argumento.                 ==
 ============================================================================*/
PRIVADA dir32 tomar_memoria(unt32 tam)
{
   struct espacio *p_actual, *p_anterior;
   dir32  direccion;
   
   /* comenzamos por el primer espacio buscando uno lo suficientemente grande
   * para contener la cantidad solicitada de memoria.
   */
   p_actual = cabeza;
   while(p_actual != NULL){
   
      /* si este espacio es lo suficientemente grande, lo usamos */
      if(p_actual->capacidad >= tam){
         direccion = p_actual->direccion;
         p_actual->direccion += tam;
         p_actual->capacidad -= tam;
   
         /* si la capacidad sobrante es 0 debemos quitar este
         * espacio de la lista.
         */
         if(p_actual->capacidad == 0) quitar_espacio(p_anterior, p_actual);
         return direccion;
      }
   
      /* si no es este, sera el proximo o no? */
      p_anterior = p_actual;
      p_actual   = p_actual->prox;
   }
   
   return NULL;
}


/*============================================================================
 == obt_espacio                                                             ==
 == Busca un espacio de memoria libre lo suficientemente grande como para   ==
 == alojar la cantidad de byte solicitados en el argumento.                 ==
 ============================================================================*/
PRIVADA void liberar_memoria(dir32 base, unt32 cant)
{
   struct espacio *np, *pe, *ant;
   
   if(cant <= 0) return;
   // if(libre == NULL)
   np = libre;             /* np es el puntero a la entrada de espacios libres */
   np->direccion = base;   /* pongo los datos del nuevo espacio libre */
   np->capacidad = cant;
   libre = np->prox;       /* libre debe apuntar ahora al proximo espacio libre*/
   
   pe = cabeza;
   
   /* Si la direccion de este bloque es menor que el actualmente menor disponible
   * o si no hay espacios libres, ponemos la entrada al espacio libre al principio
   * de la lista.
   */
   if(pe == NULL || base <= pe->base) {
      np->prox = pe;
      cabeza   = np;
      merge(np);
      return;
   }
   
   /* Si no ocurrio el caso del comentario de arriba, entonces el puntero pe debe
   *  apuntar a la entrada cuya direccion base sea mayor a la direccion base para
   *  liberar.
   */
   for(;pe != NULL && base > pe->base; pe = pe->prox, ant = pe);
   
   /* Una vez que encontramos la entrada adecuada, insertamos el bloque en la posicion
   *  anterior y comprobamos si no hay espacios contiguos que se puedan fusionar para
   *  hacerlos uno   solo y de esta manera liberar una entrada en la tabla de espacios
   */
   np->prox  = ant->prox;
   ant->prox = np;
   merge(ant);
}




/*===========================================================================
 ==                              del_slot                                  ==  
 ===========================================================================*/
PRIVADA void quitar_espacio(struct espacio *p_anterior, struct espacio *p_actual)
{
   if(p_actual == cabeza)
      cabeza   = p_actual->prox;
   else
      p_anterior->prox = p_actual->prox;

   p_actual->prox = libre;
   libre = p_actual;
}


/*===========================================================================*
 *                              merge                                        *
 *===========================================================================*/
PRIVATE void unir_entradas(struct espacio * p_actual)
{
   struct espacio * p_proximo;
   
   /* Si el puntero 'p_actual' apunta a la ultima entrada de la lista, entonces
   *  no tiene una entrada proxima para unirlo. Si este no es el caso, vemos si
   *  entre los dos espacios de memoria no hay memoria ocupada. Si son contiguos
   *  entonces hacemos que la entrada apuntada por 'p_actual' tenga el tamaño de
   *  los dos y quitamos la entrada siguiente 'p_proximo'.
   */
   p_proximo = p_actual->prox;
   if( p_proximo == NULL) return;
   if( p_actual->base + p_actual->capacidad == p_proximo->base){
      p_actual->capacidad += p_proximo->capacidad;   /* first one gets second one's mem */
      quitar_espacio(p_actual, p_proximo);
   } else {
      p_actual = p_proximo;
   }
   
   /* Probamos con la siguiente entrada de igual manera que lo hicimos arriba */
   p_proximo = p_actual->prox;
   if( p_proximo == NULL) return;
   if( p_actual->base + p_actual->capacidad == p_proximo->base) {
      p_actual->capacidad += p_proximo->capacidad;   /* first one gets second one's mem */
      quitar_espacio(p_actual, p_proximo);
   }
}


/*===========================================================================*
 *                              mem_init                                     *
 *===========================================================================*/
PUBLIC void mem_init(total, free)
phys_clicks *total, *free;              /* memory size summaries */
{
/* Initialize hole lists.  There are two lists: 'hole_head' points to a linked
 * list of all the holes (unused memory) in the system; 'free_slots' points to
 * a linked list of table entries that are not in use.  Initially, the former
 * list has one entry for each chunk of physical memory, and the second
 * list links together the remaining table slots.  As memory becomes more
 * fragmented in the course of time (i.e., the initial big holes break up into
 * smaller holes), new table slots are needed to represent them.  These slots
 * are taken from the list headed by 'free_slots'.
 */

   struct espacio * p_actual, *p_ultimo;
   dir32 base;
   unt32 tam;
   mensaje m;
   
   cabeza   = NULL;
   libre    = &espacios[0];
   p_ultimo = &espacios[NR_ESPACIOS-1];
   p_ultimo->prox = NULL;
   
   
   /* Ponemos todos las entradas en la lista libre */
   p_actual = libre;
   while(p_actual <= p_ultimo){
      p_actual->prox = p_actual + 1;
      p_actual++;
   }
   
   
   /* Ahora le preguntamos al kernel por los espacios de memoria que tiene
   * disponible y por cada uno de ellos los registro en la lista.
   */
   for (;;){
      m.m_type = SYS_MEM;
      base = m.m1_i1;
      tam  = m.m1_i2;
      if(tam == 0) break;
   
      liberar_memoria(base, tam);
   }
}
