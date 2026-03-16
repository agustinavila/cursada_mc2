# src

Contiene el codigo propio del firmware.

Subcarpetas principales:
- `app/`: orquestacion principal, parametros y persistencia
- `control/`: estrategias de control
- `drivers/`: drivers propios del proyecto
- `hmi/`: interfaz de usuario sobre LCD y pulsadores

El arranque bare-metal del LPC4337 ya no vive aca: se movio a `platform/lpc4337/startup/` para separar mejor bootstrap de aplicacion.
