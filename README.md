# cursada_mc2

Entorno reproducible para la EDU-CIAA-NXP (LPC4337, core M4) en Windows usando VS Code, CMake, OpenOCD y GDB, sin PlatformIO ni Eclipse.

## Indice

- [Funcionamiento general](#funcionamiento-general)
- [Uso de la HMI](#uso-de-la-hmi)
- [Estructura](#estructura)
- [Prerrequisitos en Windows](#prerrequisitos-en-windows)
- [Targets disponibles](#targets-disponibles)
- [Como mantener la configuracion de CMake](#como-mantener-la-configuracion-de-cmake)
- [Build desde terminal](#build-desde-terminal)
- [Flash desde terminal](#flash-desde-terminal)
- [Debug en VS Code](#debug-en-vs-code)
- [SVD y perifericos en debug](#svd-y-perifericos-en-debug)
- [Validacion de OpenOCD y del probe FTDI](#validacion-de-openocd-y-del-probe-ftdi)
- [Rol de LPCOpen en esta base](#rol-de-lpcopen-en-esta-base)
- [Startup, linker y system init](#startup-linker-y-system-init)
- [Sensores DS18B20 por 1-Wire](#sensores-ds18b20-por-1-wire)
- [Troubleshooting](#troubleshooting)
- [Validacion minima sugerida](#validacion-minima-sugerida)
- [Artefactos legacy removidos](#artefactos-legacy-removidos)

## Funcionamiento general

La aplicacion actual implementa una base para control de temperatura sobre la EDU-CIAA-NXP.

Un caso de uso concreto para esta base es el control de temperatura durante la
fermentacion de cerveza. En ese proceso, mantener la temperatura dentro de un
rango estable es importante porque afecta directamente la actividad de la
levadura, la velocidad de fermentacion y el perfil final de aromas y sabores.
Una temperatura demasiado alta puede generar subproductos no deseados y una
temperatura demasiado baja puede frenar o volver ineficiente la fermentacion.

La base soporta ambos modos de trabajo, calentar y enfriar, pero en este caso
el uso mas comun suele ser enfriar, porque la fermentacion es un proceso
exotermico. Eso permite, por ejemplo, habilitar la circulacion de un chiller a
traves de una serpentina cuando la temperatura supera el valor objetivo. En una
implementacion concreta, la accion de control podria activar una bomba para
forzar esa circulacion o una electroválvula que habilite el paso del fluido de
enfriamiento.

Tambien hay un caso complementario en temporadas de invierno o en ambientes muy
frios, donde la temperatura exterior puede hacer caer demasiado la temperatura
del fermentador. En esa situacion, el mismo esquema de control permite trabajar
en modo calentar para sostener la temperatura de fermentacion dentro del rango
deseado.

A grandes rasgos, el flujo es este:

- uno o mas sensores `DS18B20` miden temperatura sobre un bus `1-Wire`
- la aplicacion descubre los sensores presentes, pero por ahora usa siempre el primero encontrado como variable de proceso
- la HMI muestra el estado general del control en el LCD y permite editar sus parametros principales con cuatro pulsadores
- sobre esa medicion corre un control `on/off` con histeresis y tiempos minimos de encendido/apagado
- la salida del control se refleja hoy en `LED1` como actuador de prueba

### Proceso de control

El lazo implementado hoy es un control `on/off`. Eso significa que la salida no
trabaja de manera proporcional, sino que solo tiene dos estados posibles:
encendida o apagada. El controlador compara la temperatura medida contra el
`setpoint` y decide si debe activar o desactivar la salida segun el modo de
trabajo:

- en `calentar`, la salida se activa cuando la temperatura cae por debajo del
  rango permitido y se desactiva al recuperar la temperatura objetivo
- en `enfriar`, la salida se activa cuando la temperatura supera el rango
  permitido y se desactiva al volver al objetivo

Para evitar que la salida conmute continuamente alrededor del `setpoint`, el
control usa `histeresis`. La histeresis define una banda de tolerancia alrededor
del valor objetivo y evita que pequeñas variaciones o ruido en la medicion
produzcan encendidos y apagados repetidos. Sin histeresis, si la temperatura se
mantuviera oscilando muy cerca del setpoint, la salida podria cambiar de estado
demasiado seguido.

Ademas, el control incorpora tiempos minimos de encendido y apagado. Estos
delays no bloquean el lazo principal, sino que obligan a que la salida permanezca
un tiempo minimo en cada estado antes de permitir una nueva conmutacion. Esto
ayuda a filtrar cambios rapidos debidos a ruido, mediciones inestables o
fluctuaciones transitorias del proceso. En una aplicacion real tambien sirve
para proteger actuadores que no conviene conmutar demasiado seguido, como un
compresor, una electroválvula o un relé.

En el estado actual:

- el sensor de proceso esta fijado al indice `0`
- la logica de sensores vive en `app`, no en la HMI
- el control implementado hoy es un unico lazo `on/off`

## Futuras funciones posibles

Esta base hoy esta enfocada en un solo lazo de control simple, pero hay varias
extensiones razonables para etapas futuras del proyecto:

- control de multiples lazos en simultaneo
  - soporte para varios sensores y varios actuadores trabajando en paralelo
  - util si se quiere controlar mas de un fermentador, o distintas zonas
    termicas dentro de un mismo sistema

- seleccion explicita del sensor de proceso
  - hoy la app usa siempre el primer sensor detectado
  - una mejora posible es permitir elegir desde la HMI que sensor usar como
    variable de control

- alarmas de proceso
  - alarmas por temperatura alta (`HI`)
  - alarmas por temperatura baja (`LO`)
  - alarma de error o perdida de sensor
  - estas alarmas podrian reflejarse en el LCD, en un buzzer o en una salida
    dedicada

- salidas separadas para calentar y enfriar
  - en vez de una unica salida logica, el sistema podria manejar una salida
    dedicada para calefaccion y otra para enfriamiento
  - esto es util cuando ambos actuadores existen en el mismo equipo

- nuevos modos de control
  - ademas del `on/off`, se podria agregar un control proporcional o `PI`
  - eso permitiria una regulacion mas fina en procesos con mayor inercia o con
    requerimientos mas exigentes de estabilidad

- salidas moduladas por `PWM`
  - una salida `PWM` puede ser util cuando el actuador admite modulacion en vez
    de simple conmutacion
  - por ejemplo, para regular la potencia de una resistencia calefactora a
    traves de una etapa de potencia adecuada, o para modular la velocidad de una
    bomba o ventilador si el hardware asociado lo permite

- mejoras generales de supervision
  - registro de eventos o historico basico de temperaturas
  - configuracion mas completa desde HMI
  - diagnostico de sensores y actuadores
  - integracion futura con telemetria o supervisión externa

## Uso de la HMI

La interfaz de usuario usa el LCD de 16x2 y los cuatro pulsadores del poncho.

### Pantalla principal

La pantalla principal muestra el estado general del sistema:

- la linea superior muestra temperatura actual, estado de salida y modo de control
- la linea inferior muestra `SP` e `H`
- si no hay un sensor valido, la temperatura se muestra como `--.-C`

### Pulsadores

La navegacion actual se hace asi:

- `Tecla 1`: entra o sale del menu
- `Tecla 2`: sube o incrementa
- `Tecla 3`: baja o decrementa
- `Tecla 4`: entra en una opcion o confirma edicion

Por ahora no hay una accion especial asociada a mantener una tecla presionada.

### Menu actual

La HMI implementa un menu jerarquico simple. En el LCD:

- la linea superior indica si estas en `MENU`, en un submenu o en modo `EDITAR`
- la linea inferior muestra la opcion actual o el valor que se esta editando

Menu actual:

- `Param control`
  - `Setpoint`
  - `Histeresis`
  - `Modo`

Los cambios de parametros se aplican solo al confirmar con `Enter`. Si se sale de la edicion con `Menu`, el valor temporal se descarta.

## Estructura

- `src/`: codigo propio del firmware.
  Incluye aplicacion, control, HMI y drivers propios en `src/drivers/`.
- `src/startup/`: arranque bare-metal del M4, `SystemInit` y CRP.
- `third_party/lpcopen/chip_43xx/`: codigo vendor heredado de LPCOpen.
  En esta base se usa solo como chip support layer del LPC4337.
- `platform/ldscripts/default/`: fragmentos de linker script heredados de NXP/MCUXpresso para el LPC4337.
- `platform/svd/LPC43xx_43Sxx.svd`: descripcion SVD usada por el debugger para registros y perifericos.
- `platform/openocd/ciaa-nxp.cfg`: configuracion de OpenOCD validada para la interfaz FTDI/JTAG de la EDU-CIAA-NXP.
- `cmake/`: toolchain y helpers de CMake para bare-metal.
- `.vscode/`: tasks, launch y settings para VS Code.

En esta etapa LPCOpen se movio a `third_party/lpcopen/` para dejar mas claro que es codigo vendor. Los linker scripts y OpenOCD se mantienen en `platform/` porque forman parte de la integracion especifica con la placa.

Criterio general de organizacion:

- `src/`
  - Codigo propio del firmware: aplicacion, HMI, control, drivers y startup.
- `third_party/`
  - Codigo vendor que el proyecto reutiliza sin modificar internamente, como LPCOpen.
- `platform/`
  - Archivos de soporte especificos de la integracion con la placa y el flujo de tooling.

Si en el futuro aparece otro archivo necesario para linker, flashing o debug de la placa, deberia agregarse en `platform/` antes que en `src/`.

Ademas del README principal, las carpetas mas importantes ya incluyen notas cortas para orientar el recorrido del repo:

- `src/`
- `src/app/`
- `src/control/`
- `src/drivers/`
- `src/hmi/`
- `src/startup/`
- `third_party/lpcopen/`

## Prerrequisitos en Windows

Estas herramientas tienen que estar accesibles desde `PATH`:

- `cmake`
- `ninja`
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `arm-none-eabi-objdump`
- `openocd`

### Descargas sugeridas para Windows

Enlaces oficiales, alineados con las versiones validadas en este repo cuando fue posible:

- `VS Code`
  - descarga general: [code.visualstudio.com](https://code.visualstudio.com/download)
- `CMake`
  - descarga general: [cmake.org/download](https://cmake.org/download/)
  - version validada en este repo: [CMake 3.27.6 para Windows x64](https://github.com/Kitware/CMake/releases/download/v3.27.6/cmake-3.27.6-windows-x86_64.msi)
- `Ninja`
  - pagina oficial del proyecto: [github.com/ninja-build/ninja/releases](https://github.com/ninja-build/ninja/releases)
  - version validada en este repo: [Ninja 1.11.1 para Windows](https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip)
- `Arm GNU Toolchain`
  - descarga general: [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
  - version validada en este repo: [Arm GNU Toolchain 14.2.Rel1](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/14-2-rel1)
- `OpenOCD`
  - release alineada con este repo: [xPack OpenOCD v0.12.0-7](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.12.0-7)
  - para Windows, bajar el ZIP de la release, descomprimirlo en una carpeta simple como `C:\OpenOCD\` y agregar `C:\OpenOCD\bin` al `PATH`

Herramienta opcional:

- `cppcheck`
  - pagina oficial: [cppcheck.sourceforge.io](https://cppcheck.sourceforge.io/)
  - version validada en esta maquina: `2.20.0`

Versiones validadas en esta maquina para el flujo actual:

- `cmake`: `3.27.6`
- `ninja`: `1.11.1`
- `arm-none-eabi-gcc`: `Arm GNU Toolchain 14.2.Rel1`, `gcc 14.2.1 20241119`
- `arm-none-eabi-gdb`: `Arm GNU Toolchain 14.2.Rel1`, `gdb 15.2.90.20241130-git`
- `arm-none-eabi-objdump`: `Arm GNU Toolchain 14.2.Rel1`, `objdump 2.43.1.20241119`
- `openocd`: `0.12.0 (2023-01-14-23:37)`

### Herramientas que deben quedar en PATH

Para que CMake, VS Code, OpenOCD y las tasks del proyecto funcionen sin rutas absolutas, estas herramientas deben poder ejecutarse directamente desde una terminal:

- `cmake`
- `ninja`
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `arm-none-eabi-objdump`
- `openocd`

Opcional:

- `cppcheck`

En la practica, lo que suele agregarse al `PATH` no es el ejecutable individual sino la carpeta `bin` de cada herramienta.

Ejemplos tipicos:

- `C:\Program Files\CMake\bin`
- `C:\Program Files\Ninja`
- `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin`
- `C:\OpenOCD\bin`
- `C:\Program Files\Cppcheck`

### Como agregar herramientas al PATH en Windows

1. Abrir `Inicio`.
2. Buscar `Editar las variables de entorno del sistema`.
3. Entrar en `Variables de entorno`.
4. En `Variables de usuario` o `Variables del sistema`, elegir `Path`.
5. Hacer click en `Editar`.
6. Agregar una entrada por cada carpeta `bin` o carpeta de ejecutables.
7. Aceptar todas las ventanas.
8. Cerrar y volver a abrir VS Code o la terminal para que el cambio tome efecto.

Despues de eso, conviene verificar:

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
arm-none-eabi-objdump --version
openocd --version
```

Y si tambien queres usar analisis estatico local:

```powershell
cppcheck --version
```

Con estas versiones se validaron:

- configuracion `debug` y `release`
- build de `cursada_mc2_app`
- flash con OpenOCD
- sesion de GDB detenida en `main`

Si el toolchain no esta en `PATH`, podes configurar una ruta explicita al momento de configurar. Usa la carpeta raiz del toolchain o su subcarpeta `bin`, segun tu instalacion:

```powershell
cmake --preset debug -DARM_NONE_EABI_TOOLCHAIN_PATH="<ruta-al-toolchain-o-bin>"
```

## Targets disponibles

- `cursada_mc2_app`: aplicacion actual con los drivers existentes.

Cada target genera:

- `*.elf`
- `*.bin`
- `*.hex`

Los artefactos quedan en `build/debug/` o `build/release/`.

Uso previsto de cada formato:

- `*.elf`: debug con simbolos, GDB y VS Code
- `*.bin`: programacion de flash con OpenOCD
- `*.hex`: artefacto alternativo para programacion o distribucion

## Documentacion API

El repo incluye un `Doxyfile` en la raiz para generar documentacion HTML a partir
de `README.md`, `src/` y los comentarios Doxygen del codigo.

Uso local:

```powershell
doxygen Doxyfile
```

La salida queda en:

- `build/docs/doxygen/html`

GitHub Actions:

- el workflow `Doxygen Docs` genera la documentacion en cada PR contra `main`
- al hacer `push` a `main`, publica automaticamente la documentacion en `GitHub Pages`

Paso manual necesario en GitHub:

1. abrir `Settings -> Pages`
2. en `Build and deployment`, elegir `Source: GitHub Actions`

Una vez habilitado, la documentacion quedara publicada desde el sitio de Pages del repo.

## Como mantener la configuracion de CMake

La idea de esta base es que los archivos de CMake se puedan seguir sin conocer demasiado la herramienta. Cada archivo tiene un rol concreto:

- `CMakeLists.txt`
  - configura el proyecto completo
  - define la biblioteca comun `lpc43xx_common`
  - agrega el codigo vendor y el subdirectorio `src/`
- `src/CMakeLists.txt`
  - define el startup del micro
  - arma el ejecutable `cursada_mc2_app`
  - lista los modulos de aplicacion, control y HMI que se compilan
- `src/drivers/CMakeLists.txt`
  - lista los drivers propios que forman la biblioteca `cursada_mc2_drivers`
- `cmake/lpc4337.cmake`
  - concentra la configuracion especifica del LPC4337
  - genera el linker script efectivo
  - configura `.elf`, `.bin`, `.hex` y el target de flash con OpenOCD
- `cmake/toolchain-arm-none-eabi.cmake`
  - busca el toolchain `arm-none-eabi-*`
  - define como CMake encuentra compilador y herramientas auxiliares
- `arm-none-eabi-gcc.cmake`
  - shim legacy mantenido por compatibilidad
  - el entrypoint oficial del toolchain es `cmake/toolchain-arm-none-eabi.cmake`
- `CMakePresets.json`
  - define los presets `debug` y `release`
  - indica en que carpeta se construye cada configuracion

### Que archivo tocar segun el cambio

Si agregas o eliminas codigo propio, normalmente alcanza con editar uno de estos dos archivos:

- nuevo modulo de aplicacion, HMI o control:
  - agregar o quitar el `.c` en `src/CMakeLists.txt`
- nuevo driver propio:
  - agregar o quitar el `.c` en `src/drivers/CMakeLists.txt`

Ejemplo: si agregas `src/control/control_pi.c`, tenes que sumarlo a la lista `CURSADA_MC2_APP_SOURCES` en `src/CMakeLists.txt`.

Ejemplo: si agregas `src/drivers/eeprom_driver.c`, tenes que sumarlo a la lista `CURSADA_MC2_DRIVER_SOURCES` en `src/drivers/CMakeLists.txt`.

### Cuando hace falta tocar otros archivos

- cambiar flags generales, includes o defines comunes:
  - revisar `CMakeLists.txt`
- cambiar que parte de LPCOpen se compila:
  - revisar `third_party/lpcopen/chip_43xx/CMakeLists.txt`
- cambiar formato de salida, linker o comando de flash:
  - revisar `cmake/lpc4337.cmake`
- cambiar como se encuentra el toolchain:
  - revisar `cmake/toolchain-arm-none-eabi.cmake`
- cambiar carpetas de build o presets:
  - revisar `CMakePresets.json`

### Flujo recomendado despues de tocar CMake

Cada vez que agregues, quites o muevas fuentes en CMake, conviene repetir:

```powershell
cmake --preset debug
cmake --build --preset debug --target cursada_mc2_app
```

Si tambien cambiaste tooling o integracion local, conviene correr ademas:

```powershell
cppcheck --template=gcc --enable=warning,style,performance,portability --error-exitcode=1 --inline-suppr "--suppress=missingIncludeSystem" "--suppress=constParameterPointer:third_party/lpcopen/chip_43xx/inc/*" -D__GNUC__ -DCORE_M4 -Isrc -Isrc/drivers -Ithird_party/lpcopen/chip_43xx/inc src/main.c src/startup/sysinit.c src/hmi src/app src/control src/drivers
```

Como alternativa, el proyecto expone un target opcional de CMake:

```powershell
cmake --build --preset debug --target cppcheck
```

Si `cppcheck` no esta instalado en la maquina, ese target no falla: solo informa que el analisis se omite.

## Build desde terminal

Configurar y compilar en debug:

```powershell
cmake --preset debug
cmake --build --preset debug --target cursada_mc2_app
```

Release:

```powershell
cmake --preset release
cmake --build --preset release --target cursada_mc2_app
```

## Flash desde terminal

Aplicacion actual:

```powershell
cmake --build --preset debug --target flash_cursada_mc2_app
```

Ese target:

- genera antes el `*.bin` desde el `*.elf`
- programa la flash principal en `0x1A000000`
- verifica sobre el `*.bin`

Se usa `*.bin` en vez de programar el `*.elf` directamente porque el flujo es mas robusto con OpenOCD en LPC43xx, especialmente cuando el ELF tiene secciones con direccion de ejecucion en RAM.

OpenOCD usa siempre:

```powershell
openocd -f platform/openocd/ciaa-nxp.cfg
```

## Debug en VS Code

Extensiones recomendadas:

- `ms-vscode.cmake-tools`
- `marus25.cortex-debug`
- `ms-vscode.cpptools`

Flujo:

1. Ejecutar `Configure [debug]` o usar el preset `debug` de CMake Tools.
2. Ejecutar `Build App [debug]` o `Build + Flash App [debug]`.
3. Si queres programar la placa sin abrir una sesion de debug, ejecutar `Flash App [debug]`.
4. Elegir `Debug App (OpenOCD)` en la pestana Run and Debug.
5. El debugger usa `runToEntryPoint: main`, asi que debe detenerse en `main`.

La configuracion de debug sigue usando el `*.elf`, mientras que el target de flash de CMake usa el `*.bin`.

Tambien hay un perfil complementario:

- `Attach App (OpenOCD)`

Ese perfil sirve para adjuntarse a una placa ya programada sin relanzar la carga desde VS Code. Es util cuando primero queres flashear desde terminal o task de CMake y despues abrir una sesion GDB sobre ese firmware.

## SVD y perifericos en debug

La configuracion de VS Code referencia:

- `platform/svd/LPC43xx_43Sxx.svd`

Ese archivo habilita la vista de perifericos y registros en `cortex-debug`.

Estado actual:

- es util para inspeccion de registros de la familia LPC43xx
- se mantiene integrado en `launch.json`
- no debe asumirse como una descripcion exacta y perfecta del `LPC4337` concreto

Si mas adelante se consigue un `SVD` mas preciso para el LPC4337/M4F, conviene reemplazarlo sin cambiar el flujo de debug.

## Validacion de OpenOCD y del probe FTDI

Validacion minima de conectividad:

```powershell
openocd -f platform/openocd/ciaa-nxp.cfg -c "init; targets; shutdown"
```

Ese comando deberia detectar al menos el TAP `lpc4337.m4`.

Para inspeccionar dispositivos USB/driver en Windows:

```powershell
Get-PnpDevice -PresentOnly | Where-Object { $_.FriendlyName -match 'FTDI|CMSIS|J-Link|ST-Link|LPC-Link|NXP|CIAA' } | Format-Table -Auto
```

Si necesitas ver mas detalle del FTDI:

```powershell
pnputil /enum-devices /connected
```

Buscar la interfaz FTDI con `VID 0403` y `PID 6010`.

## Rol de LPCOpen en esta base

LPCOpen queda reducido a vendor code de soporte:

- CMSIS headers
- chip support
- drivers de chip realmente usados por el firmware actual

No se usa board library. El codigo propio vive en `src/` y se apoya en `lpc_chip_43xx` como capa de bajo nivel.

En esta etapa:

- no se tocaron archivos internos de LPCOpen
- se redujo desde CMake la lista de fuentes vendor compiladas a las efectivamente necesarias
- se reubico LPCOpen en `third_party/lpcopen/chip_43xx/` para dejar mas claro su rol de vendor

## Startup, linker y system init

La base actual conserva estos archivos como parte del arranque bare-metal del M4:

- `src/startup/cr_startup_lpc43xx.c`
- `src/startup/sysinit.c`
- `src/startup/crp.c`
- `platform/ldscripts/default/mem/mem.ld`
- `platform/ldscripts/default/sections/sections.ld`

Origen y criterio:

- provienen de la base NXP/MCUXpresso/LPCOpen
- estan integrados a CMake y validados con esta placa
- se conservan por compatibilidad y estabilidad

No se reemplazaron en esta etapa porque hoy no son el problema principal del repo y ya resuelven correctamente:

- vector table
- inicializacion temprana
- `SystemInit`
- layout de memoria
- enlace del firmware bare-metal

## Sensores DS18B20 por 1-Wire

La base actual incluye soporte para sensores `DS18B20` conectados sobre un bus `1-Wire` implementado por bit-banging sobre GPIO.

### Conexionado validado

Conexionado probado sobre el Poncho Educativo UNSJ:

- bus `1-Wire` en `P8.GPIO0`
- mapeo interno: `P6_1 / GPIO3[0]`
- `DQ` del sensor -> `P8.GPIO0`
- `VDD` del sensor -> `3.3V`
- `GND` del sensor -> `GND`
- resistencia `4.7 kOhm` entre `DQ` y `3.3V`

Alimentacion soportada y documentada:

- modo alimentado normal a `3.3V`
- no se usa `parasite power`

### Comportamiento actual del firmware

La aplicacion usa el bus `1-Wire` al iniciar:

- descubre sensores `DS18B20` presentes en el bus
- si no hay sensores validos, el control queda sin medicion
- si hay uno o mas sensores, por ahora usa siempre el primero encontrado como sensor de proceso
- la HMI muestra la temperatura de ese sensor de proceso en la pantalla principal
- el estado de salida del control se refleja en `LED1`

### Alcance actual del driver

- `onewire_driver`: implementa el bus `1-Wire` por bit-banging sobre GPIO
- `ds18b20_driver`: soporta lectura de un sensor, seleccion por `ROM` y gestion de multiples sensores sobre un mismo bus
- la aplicacion usa conversion no bloqueante para no frenar el lazo principal mientras espera la conversion del sensor

### Limitaciones actuales

- cantidad maxima de sensores por bus: `DS18B20_MAX_DEVICES`
- solo se consideran sensores con codigo de familia `0x28`
- todos los sensores comparten un unico pin fisico de bus
- aunque el driver soporta multiples sensores, la app usa solo el primero detectado

### Archivos relevantes

- [src/drivers/onewire_driver.h](e:/Users/agust/Documents/cursada_mc2/src/drivers/onewire_driver.h)
- [src/drivers/ds18b20_driver.h](e:/Users/agust/Documents/cursada_mc2/src/drivers/ds18b20_driver.h)
- [src/app/app.c](e:/Users/agust/Documents/cursada_mc2/src/app/app.c)
- [src/hmi/hmi.c](e:/Users/agust/Documents/cursada_mc2/src/hmi/hmi.c)

### Prueba en hardware

1. Conectar uno o mas sensores `DS18B20` al mismo bus `P8.GPIO0`.
2. Verificar que el bus tenga la resistencia pull-up de `4.7 kOhm`.
3. Compilar la app:

```powershell
cmake --build --preset debug --target cursada_mc2_app
```

4. Flashear la app:

```powershell
cmake --build --preset debug --target flash_cursada_mc2_app
```

5. Observar la pantalla principal:
   - sin sensores: temperatura invalida y salida inactiva
   - con uno o mas sensores: el firmware usa el primero detectado como sensor de proceso

### Troubleshooting DS18B20

- `No detectado`: revisar cableado, `3.3V`, `GND`, `DQ` y la resistencia pull-up
- lectura inestable: revisar longitud del cable y masa comun entre placa y sensores
- solo detecta uno de varios: revisar que todos compartan el mismo bus y la misma alimentacion
- OpenOCD falla al flashear: verificar que no haya un proceso `openocd` previo reteniendo la interfaz FTDI

## Troubleshooting

### OpenOCD no detecta la placa

- Verificar que la placa este alimentada y conectada.
- Probar `openocd -f platform/openocd/ciaa-nxp.cfg -c "init; targets; shutdown"`.
- Verificar si Windows esta exponiendo la interfaz FTDI correcta.
- No cambiar drivers si OpenOCD ya detecta el TAP.
- Usar Zadig y `WinUSB` solo si OpenOCD deja de abrir la interfaz FTDI o no aparece el dispositivo correcto.

### GDB no conecta

- Confirmar que `arm-none-eabi-gdb` este en `PATH`.
- Confirmar que `openocd` arranca sin errores con el mismo `cfg`.
- Revisar que el `launch.json` apunte al `.elf` del preset `debug` y que `arm-none-eabi-gdb`, `arm-none-eabi-objdump` y `openocd` esten en `PATH`.
- Si queres conectarte sin relanzar la carga desde VS Code, usar `Attach App (OpenOCD)` en vez de `Debug App (OpenOCD)`.
- Si una sesion previa dejo `openocd` abierto, cerrar la sesion de debug o terminar el proceso antes de reintentar.

### VS Code no encuentra el toolchain

- Verificar `arm-none-eabi-gcc --version`.
- Si no esta en `PATH`, configurar `ARM_NONE_EABI_TOOLCHAIN_PATH` al correr `cmake --preset debug`.
- Reconfigurar el preset despues del cambio.

### Problemas con rutas en Windows

- Esta base usa `build/debug` y `build/release`, no el arbol legado `Debug/`.
- Si moves el repo de carpeta, reconfigura con `cmake --preset debug`.
- No reutilices `CMakeCache.txt` viejos de otra ruta.

### Errores de linker o startup

- Confirmar que el target sea el core M4.
- Confirmar que no se haya cambiado el linker script base bajo `platform/ldscripts/default/`.
- Confirmar que `src/startup/cr_startup_lpc43xx.c` y `src/startup/sysinit.c` esten siendo compilados.

### Problemas por dependencias viejas de LPCOpen

- Evitar agregar codigo nuevo que dependa del board library.
- Mantener el acceso a perifericos a traves de drivers propios o directamente sobre `chip.h`.
- Si aparece una dependencia a simbolos de MCUXpresso/Code Red, revisar primero si viene de headers viejos o includes heredados.

## Validacion minima sugerida

1. Compilar `cursada_mc2_app`.
2. Verificar `build/debug/cursada_mc2_app.elf`, `.bin` y `.hex`.
3. Flashear `cursada_mc2_app`.
4. Abrir `Debug App (OpenOCD)` en VS Code.
5. Confirmar que el debugger se detiene en `main`.

## Artefactos legacy removidos

Esta base ya no trackea artefactos del flujo Eclipse/MCUXpresso que no participan del entorno reproducible actual, por ejemplo:

- `.project`
- `.cproject`

El flujo soportado del repo usa exclusivamente:

- `build/debug`
- `build/release`
- VS Code
- CMake presets
- OpenOCD
- arm-none-eabi-gdb

El ejemplo legacy `periph_uart` no forma parte del flujo reproducible actual y debe tratarse como material viejo fuera del firmware principal.
