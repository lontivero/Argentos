OBJS= debug.o

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








