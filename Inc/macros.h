/***********************************************************************************************
*   macros.h
************************************************************************************************
*   Declaracion macros para no lidiar con el ensamblador
**********************************************************************************************/
#ifndef _MSO_MACROS_H_
#define _MSO_MACROS_H_


#include "tipos.h"



static __inline__ unt8 inp8(unt16 _p_)
{
	unt8 _s_;
	
	__asm__ __volatile__("inb %[puerto],%[salida]"
	  : [salida] "=a" (_s_)
	  : [puerto] "d"  (_p_));
	
	return _s_;
}


static __inline__ unt16 inp16(unt16 _p_)
{
	unt16 _s_;
	
	__asm__ __volatile__("inw %[puerto],%[salida]"
	  : [salida] "=a" (_s_)
	  : [puerto] "d"  (_p_));
	
	return _s_;
}


static __inline__ unt32 inp32(unt16 _p_)
{
	unt32 _s_;
	
	__asm__ __volatile__("inl %[puerto],%[salida]"
	  : [salida] "=a" (_s_)
	  : [puerto] "d"  (_p_));
	
	return _s_;
}



static __inline__ void outp8(unt16 _p_, unt8 _d_)
{
	__asm__ __volatile__("outb %[dato],%[puerto]"
	  : : [puerto] "d" (_p_), [dato] "a" (_d_));
}


static __inline__ void outp16(unt16 _p_, unt16 _d_)
{
	__asm__ __volatile__("outw %[dato],%[puerto]"
	  : : [puerto] "d" (_p_), [dato] "a" (_d_));
}

static __inline__ void outp32(unt16 _p_, unt32 _d_)
{
	__asm__ __volatile__("outl %[dato],%[puerto]"
	  : : [puerto] "d" (_p_), [dato] "a" (_d_));
}


static __inline__ void _DS(unt16 selector)
{
	__asm__ __volatile__
	  ("movw %w0,%%ds"
	  :: "rm" (selector));
}

static __inline__ unt16 DS()
{
	unt16	selector;
	__asm__ __volatile__
	  ("movw %%ds,%w0"
	  : "=rm" (selector)
	  : );

	return selector;
}

static __inline__ void _ES(unt16 selector)
{
	__asm__ __volatile__
	  ("movw %w0,%%es"
	  :: "rm" (selector));
}

static __inline__ unt16 ES()
{
	unt16	selector;
	__asm__ __volatile__
	  ("movw %%es,%w0"
	  : "=rm" (selector)
	  : );

	return selector;
}

static __inline__ void _FS(unt16 selector)
{
	__asm__ __volatile__
	  ("movw %w0,%%fs"
	  :: "rm" (selector));
}

static __inline__ unt16 FS()
{
	unt16	selector;
	__asm__ __volatile__
	  ("movw %%fs,%w0"
	  : "=rm" (selector)
	  : );

	return selector;
}

static __inline__ void _GS(unt16 selector)
{
	__asm__ __volatile__
	  ("movw %w0,%%gs"
	  :: "rm" (selector));
}

static __inline__ unt16 GS()
{
	unt16	selector;
	__asm__ __volatile__
	  ("movw %%gs,%w0"
	  : "=rm" (selector)
	  : );

	return selector;
}


/**/
static __inline__ unt32 CR3(){
	unt32 ret;

	__asm__ __volatile__
	  ("movl %%cr3,%0"
	  :"=r" (ret)
	  : );
	return ret;
}

static __inline__ void _CR3(unt32 _cr3){
	__asm__ __volatile__
	  ("movl %0, %%cr3"
	  :
	  :"r" (_cr3) );
}


static __inline__ unt32 CR2(){
	unt32 ret;

	__asm__ __volatile__
	  ("movl %%cr2,%0"
	  :"=r" (ret)
	  : );
	return ret;
}

static __inline__ void _CR2(unt32 _cr2){
	__asm__ __volatile__
	  ("movl %0, %%cr2"
	  :
	  :"r" (_cr2) );
}

static __inline__ unt32 CR0(){
	unt32 ret;

	__asm__ __volatile__
	  ("movl %%cr0,%0"
	  :"=r" (ret)
	  : );
	return ret;
}

static __inline__ void _CR0(unt32 _cr0){
	__asm__ __volatile__
	  ("movl %0, %%cr0"
	  :
	  : "r" (_cr0) );
}




static __inline__ void activar(void)
{	__asm__ __volatile__("sti"::);	}

static __inline__ unt32 desactivar(void)
{
	unt32 ret;

	__asm__ __volatile__(
	  "pushfl\n"
	  "popl %0\n"
	  "cli"
	  : "=a"(ret)
	  : );
	return ret;
}

static __inline__ void sflags(unt32 flags)
{	__asm__ __volatile__(
	   "pushl %0\n"
	   "popfl"
	   :
	   : "m"(flags)
	);
}

static __inline__ void ltr(unt16 sel) 
{
    __asm__ __volatile__ ("ltr %0": :"r" (sel));
}


#define SIDT(x)	__asm__ __volatile__("sidt %0" : "=m" (x) :	: "memory" );
#define SGDT(x)	__asm__ __volatile__("sgdt %0" : "=m" (x) :	: "memory" );
#define LIDT(x)	__asm__ __volatile__("lidt %0" : : "m" (x) 	: "memory" );
#define LGDT(x)	__asm__ __volatile__("lgdt %0" : : "m" (x) 	: "memory" );
	




#endif
