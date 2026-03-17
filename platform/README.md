# platform

Esta carpeta agrupa los artefactos de plataforma del proyecto para la
EDU-CIAA-NXP con LPC4337. A diferencia de `src/`, aca no vive la logica de
aplicacion: aca quedan los archivos necesarios para enlazar, programar y
depurar el firmware sobre la placa real.

## Contenido

- `ldscripts/`
  - Fragmentos de linker script heredados de la base NXP/MCUXpresso.
  - Definen memoria, secciones y bibliotecas de runtime usadas durante el link.
  - CMake genera a partir de estos fragmentos el linker script efectivo del
    firmware.

- `openocd/`
  - Configuracion de OpenOCD validada para la EDU-CIAA-NXP.
  - Se usa tanto desde los targets de flash de CMake como desde VS Code con
    `cortex-debug`.

- `svd/`
  - Archivos SVD usados por el debugger para mostrar perifericos y registros.
  - No participan del build del firmware; solo afectan la experiencia de debug.

Esta carpeta no contiene logica de aplicacion. Si en el futuro aparece otro
archivo necesario para linker, flashing o debug de la placa, deberia agregarse
aca antes que en `src/`.
