# app

Contiene la logica principal del firmware.

Responsabilidades actuales:
- inicializacion y ciclo principal de la aplicacion
- adquisicion del sensor DS18B20 de proceso
- sincronizacion entre HMI, parametros persistentes y control
- aplicacion de la salida de control sobre el actuador de prueba actual

Archivos relevantes:
- `app.c`: orquesta el flujo principal
- `parametros.c`: carga, valida y guarda parametros persistentes, incluyendo los defaults internos
