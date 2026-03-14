# cursada_mc2

Entorno reproducible para la EDU-CIAA-NXP (LPC4337, core M4) en Windows usando VS Code, CMake, OpenOCD y GDB, sin PlatformIO ni Eclipse.

## Estructura

- `cmake/`: toolchain y helpers de CMake para bare-metal.
- `platform/lpc43xx/`: codigo vendor heredado de NXP/LPCOpen que se conserva como capa de chip support.
- `platform/openocd/ciaa-nxp.cfg`: configuracion de OpenOCD validada para la interfaz FTDI/JTAG de la EDU-CIAA-NXP.
- `src/`: startup, system init, aplicacion actual y target minimo de validacion.
- `.vscode/`: tasks, launch y settings para VS Code.

## Prerrequisitos en Windows

Estas herramientas tienen que estar accesibles desde `PATH`:

- `cmake`
- `ninja`
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `arm-none-eabi-objdump`
- `openocd`

Verificacion rapida:

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
arm-none-eabi-objdump --version
openocd --version
```

Versiones validadas en esta maquina para el flujo actual:

- `cmake`: `3.27.6`
- `ninja`: `1.11.1`
- `arm-none-eabi-gcc`: `Arm GNU Toolchain 14.2.Rel1`, `gcc 14.2.1 20241119`
- `arm-none-eabi-gdb`: `Arm GNU Toolchain 14.2.Rel1`, `gdb 15.2.90.20241130-git`
- `arm-none-eabi-objdump`: `Arm GNU Toolchain 14.2.Rel1`, `objdump 2.43.1.20241119`
- `openocd`: `0.12.0 (2023-01-14-23:37)`

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
2. Ejecutar `Build App [debug]`.
3. Elegir `Debug App (OpenOCD)` en la pestana Run and Debug.
4. El debugger usa `runToEntryPoint: main`, asi que debe detenerse en `main`.

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
- startup/system integration alineada con LPC4337

No se usa board library. El codigo propio vive en `src/` y se apoya en `lpc_chip_43xx` como capa de bajo nivel.

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

La HMI usa el bus `1-Wire` al iniciar la app:

- descubre sensores `DS18B20` presentes en el bus
- si no hay sensores, muestra `No detectado`
- si hay un sensor, muestra `DS18B20 1/1`
- si hay varios sensores, muestra `DS18B20 X/Y` y rota automaticamente entre ellos
- la segunda linea del LCD muestra la temperatura del sensor activo

### Alcance actual del driver

- `onewire_driver`: implementa el bus `1-Wire` por bit-banging sobre GPIO
- `ds18b20_driver`: soporta lectura de un sensor, seleccion por `ROM` y gestion de multiples sensores sobre un mismo bus
- la HMI usa conversion no bloqueante para no frenar el lazo principal mientras espera la conversion del sensor

### Limitaciones actuales

- cantidad maxima de sensores por bus: `DS18B20_MAX_DEVICES`
- solo se consideran sensores con codigo de familia `0x28`
- todos los sensores comparten un unico pin fisico de bus
- la HMI rota automaticamente entre sensores; todavia no hay seleccion manual desde el menu

### Archivos relevantes

- [src/Driver/onewire_driver.h](e:/Users/agust/Documents/cursada_mc2/src/Driver/onewire_driver.h)
- [src/Driver/ds18b20_driver.h](e:/Users/agust/Documents/cursada_mc2/src/Driver/ds18b20_driver.h)
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
   - sin sensores: `No detectado`
   - con un sensor: `DS18B20 1/1`
   - con varios sensores: `DS18B20 X/Y` rotando automaticamente

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
- Confirmar que no se haya cambiado el linker script base bajo `platform/lpc43xx/ldscripts/default/`.
- Confirmar que `src/cr_startup_lpc43xx.c` y `src/sysinit.c` esten siendo compilados.

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

## Nota sobre el arbol `Debug/`

El directorio `Debug/` heredado de MCUXpresso quedo como referencia historica. El flujo reproducible actual usa exclusivamente `build/debug` y `build/release`.
