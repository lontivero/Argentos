OBJS=boot.o
TASM= ..\utiles\tasm32 /ml /m5 /l /s /la
JLOC= ..\utiles\jloc 

boot.bin: boot.asm
	$(TASM) $< 
	$(JLOC) boot.loc ..\$@ boot.map

limpiar:
	del *.obj
	del *.map
	del *.lst

dump:
	objdump -S boot.bin > dump.txt

