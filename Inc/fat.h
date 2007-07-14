#ifndef _MSO_FAT_H_
#define _MSO_FAT_H_

#define byte	unsigned char
#define word	unsigned short
#define dword	unsigned long


/* informacion del sector de booteo */
typedef struct {
    byte oemstr[8];
    word bytes_psector;
    byte sec_pcluster;
    word sec_reserv;
    byte fats;
    word entries_proot;
    word tot_sects;
    byte media_descrip;
    word sec_pfat;
    word sec_ppista;
    word cabezas;
    word sec_ocultos;
    dword tot_sectsx;
    byte drive;
    byte reserv1;
    byte signature;
    dword serial_numbre;
    byte volumen[11];
    byte reserv2[8];
} bootrec;



#endif
