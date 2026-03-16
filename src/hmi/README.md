# hmi

Contiene la interfaz de usuario sobre LCD 16x2 y pulsadores.

Responsabilidades actuales:
- render de la pantalla principal
- navegacion del menu de parametros
- edicion temporal y confirmacion de cambios

La HMI no decide el control ni adquiere sensores directamente.
Recibe desde `app` el estado visible y entrega al resto del firmware los cambios confirmados por el usuario.
