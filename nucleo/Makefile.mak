OBJS= c0.o lib.o otros.o kernel.o proc.o irq.o mprot.o excep.o reloj.o memoria.o 

all: $(OBJS)

.c.o:	
	$(CC) $< -o $*.o 

.S.o:
	$(CC) $< -o $*.o

dep:
	$(CC) -M *.c > .dep

limpiar:
	del *.o

.dep:
	touch $@

include .dep








