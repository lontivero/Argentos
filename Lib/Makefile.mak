OBJS= mem.o sform.o

all: $(OBJS)

.c.o:	
	$(CC) $< -o $*.o 
	
.asm.o:	
	$(AS386) $< -o $*.


dep:
	$(CC) -M *.c > .dep

limpiar:
	del *.o

.dep:
	touch $@

include .dep








