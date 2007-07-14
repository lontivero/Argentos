.EXPORT_ALL_VARIABLES:

# Directorios (escribe aqui tu directorio para argent-os)
DIR_RAIZ   =c:/argent-os
DIR_BOOT   =$(DIR_RAIZ)/boot
DIR_INC    =$(DIR_RAIZ)/inc
DIR_LIB    =$(DIR_RAIZ)/lib
DIR_DBG    =$(DIR_RAIZ)/dbg
DIR_NUCLEO =$(DIR_RAIZ)/nucleo
DIR_DISP   =$(DIR_NUCLEO)/disp
DIR_UTL    =$(DIR_RAIZ)/utiles
DIR_SA     =$(DIR_RAIZ)/sa

# Opciones del compilador, enlazador y del objcopy
cflags=-c -O2 -Wall -nostdlib -nostdinc -DDBG -I$(DIR_INC) 
lflags=-Map $(IMAGE).map 
oflags=-O binary -R .note -R .comment


# Herramientas GNU
OBJCOPY =objcopy $(oflags)
CC      =gcc $(cflags)
DASM    =objdump -D
MAKE    =make
AS386   =as
NM      =nm
LD      =ld


RAR     = $(DIR_UTL)/rar
MFS     = $(DIR_UTL)/mfs

# Otros
FD      = a.img
BOOT    = boot.bin
IMAGE   = kernel
OUTPUT  = $(IMAGE).bin
KERNEL_OFFSET = 0x10000


ARCHIVOS =  $(DIR_NUCLEO)/c0.o $(DIR_NUCLEO)/otros.o $(DIR_NUCLEO)/lib.o     \
            $(DIR_NUCLEO)/kernel.o $(DIR_NUCLEO)/reloj.o                     \
		$(DIR_NUCLEO)/excep.o $(DIR_NUCLEO)/mprot.o   \
		$(DIR_NUCLEO)/irq.o $(DIR_NUCLEO)/proc.o $(DIR_NUCLEO)/memoria.o \
		$(DIR_DISP)/teclado.o $(DIR_DISP)/consola.o $(DIR_DISP)/driver.o \
		$(DIR_DISP)/dram.o $(DIR_DISP)/cdf.o $(DIR_DISP)/terminal.o      \
		$(DIR_LIB)/mem.o $(DIR_LIB)/sform.o \
		$(DIR_DBG)/debug.o  \
		$(DIR_SA)/cache.o $(DIR_SA)/sa.o

all:
	$(MAKE) -C $(DIR_BOOT)   -f makefile.mak 
	$(MAKE) -C $(DIR_NUCLEO) -f makefile.mak 
	$(MAKE) -C $(DIR_DISP)   -f makefile.mak 
	$(MAKE) -C $(DIR_LIB)    -f makefile.mak 
	$(MAKE) -C $(DIR_DBG)    -f makefile.mak 
	$(MAKE) -C $(DIR_SA)     -f makefile.mak 

	$(LD) $(lflags) -o $(OUTPUT) -Ttext $(KERNEL_OFFSET) -e _start $(ARCHIVOS)
	$(NM) $(OUTPUT) > $(IMAGE).map
	$(DASM) $(OUTPUT) > $(IMAGE).txt
	$(OBJCOPY) $(OUTPUT) $(IMAGE)
	$(MFS) $(FD) -f -b$(BOOT) -C$(IMAGE)
	del $(IMAGE)
	del $(BOOT)

limpiar:
	$(MAKE) -C $(DIR_BOOT)   -f makefile.mak limpiar
	$(MAKE) -C $(DIR_NUCLEO) -f makefile.mak limpiar
	$(MAKE) -C $(DIR_DISP)   -f makefile.mak limpiar
	$(MAKE) -C $(DIR_LIB)    -f makefile.mak limpiar
	$(MAKE) -C $(DIR_DBG)    -f makefile.mak limpiar
	$(MAKE) -C $(DIR_SA)     -f makefile.mak limpiar
	del *.o
	del *.map
	del *.bin
	del *.img
	del *.txt

dep :
	$(MAKE) -C $(DIR_NUCLEO) -f makefile.mak dep
	$(MAKE) -C $(DIR_DISP)   -f makefile.mak dep
	$(MAKE) -C $(DIR_LIB)    -f makefile.mak dep
	$(MAKE) -C $(DIR_DBG)    -f makefile.mak dep
	$(MAKE) -C $(DIR_SA)     -f makefile.mak dep


bak:
	$(MAKE) -f makefile.mak limpiar
	$(RAR) a -r -m5 -MS8.zip -afzip bakup *.*
