# src

Contiene el codigo propio del firmware.

Subcarpetas principales:
- `app/`: orquestacion principal, parametros y persistencia
- `control/`: estrategias de control
- `drivers/`: drivers propios del proyecto
- `hmi/`: interfaz de usuario sobre LCD y pulsadores

La carpeta `src/startup/` contiene el arranque bare-metal del LPC4337 para dejar el bootstrap junto al resto del codigo propio, pero separado de la logica de aplicacion.
