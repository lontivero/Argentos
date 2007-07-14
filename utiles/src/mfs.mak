OBJS=  mfs.obj
TC =   \tc\bin\tcc -c -Z -1 -mt -O -d -I\tc\include
TASM=  ..\utiles\tasm32 /ml /m5 /l /s /la
TLINK= ..\utiles\tlink
E2B=   ..\utiles\exe2bin

mfs: mfs.obj
	$(TLINK) mfs, mfs

mfs.obj: mfs.c
	$(TC) $<

clean:
	del *.obj
	del *.map
