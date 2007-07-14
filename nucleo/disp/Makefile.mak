OBJS= teclado.o consola.o driver.o dram.o cdf.o terminal.o

all: $(OBJS)

.c.o:	
	$(CC) $< -o $*.o 
	
.asm.o:	
	$(AS386) $< -o $*.o


dep:
	$(CC) -M *.c > .dep

limpiar:
	del *.o

.dep:
	touch $@

include .dep








