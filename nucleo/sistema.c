/* ====================================================================== *
 ==    ++ Este archivo es parte del sistema operativo ARGENT-OS ++       ==
 ==    ++ Por: Lucas Ontivero                                            ==
 ==    ++ email: lucasontivero@yahoo.com.ar                              ==
 ======================================================================== */

#include <mensajes.h>


PRIVADA void avisar_inic_reloj();
PRIVADA int  est_alarma(mensaje *m);
PRIVADA void est_frecuencia(unt32 frec);
PRIVADA int  est_fecha_hora(mensaje *mr);
PRIVADA int  obt_fecha_hora(mensaje *mr);


/* ====================================================================== *
 ==  Configura la frecuencia de las interrupciones del reloj             ==
 ======================================================================== */
PUBLICA void t_sistema()
{
   int err;
   mensaje  m_reloj;

   avisar_inic_reloj();

   while(TRUE)
   {
      recibir(CUALQUIERA, &m_reloj);
      err = OK;

      switch(m_reloj.SERVICIO)
      {
         case OBT_FECHAHORA:
              err = obt_fecha_hora(&m_reloj);
              break;
         case EST_FECHAHORA:
              err = est_fecha_hora(&m_reloj);
              break;
         case EST_ALARMA  :
              err = est_alarma(&m_reloj);
              break;
         default:
              err = E_PARAMETRO;
      }
      
      m_reloj.RESPUESTA = err;
      enviar(m_reloj.PROC, &m_reloj);
   }
}

