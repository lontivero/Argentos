OBJS=  dsk2img.obj
TC =   \tc\bin\tcc -c -Z -1 -mt -O -d -I\tc\include
TASM=  ..\utiles\tasm32 /ml /m5 /l /s /la
TLINK= ..\utiles\tlink
E2B=   ..\utiles\exe2bin

dsk2img: dsk2img.obj
	$(TLINK) dsk2img, dsk2img

dsk2img.obj: dsk2img.c
	$(TC) $<

clean:
	del *.obj
	del *.map
