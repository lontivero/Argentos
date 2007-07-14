/***********************************************************************************************
*   mensajes.h
***********************************************************************************************/
#ifndef _MSO_MENSAJES_H_
#define _MSO_MENSAJES_H_


/* Para los mensajes	*/
#define SERVICIO	  	codigo
#define m_origen	  	quien
#define m_destino   	quien
#define m_dir       	num32_1
#define m_porigen   	num32_1
#define m_pdestino  	num32_2
#define m_cant	  	num16_1
#define m_car	  	num16_2
#define RESPUESTA		num16_2
#define PROC		quien

/* Servicios que brindan las tareas */
/* para el reloj		*/
#define OBT_FECHAHORA	1
#define EST_FECHAHORA	2
#define EST_ESPERA	3
#define EST_ALARMA	4

#define FUNC		m_dir
#define TICKS		num32_2
#define SEGUNDOS		num32_2

/* para los dispositivos */
#define ABRIR_DISP	1
#define CERRAR_DISP	2
#define IOCTL_DISP	3
#define LEER_DISP		4
#define ESCRIBIR_DISP	5


#define INTERRUPCION	1


#define DIRECCION		num32_1
#define POSICION		num32_2
#define CANTIDAD		num32_3
#define DISPOSITIVO	num16_1
#define HORA            num32_1
#define FECHA           num32_2
#define DIA_SEMANA      num16_1

#endif

