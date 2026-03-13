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
- build de `cursada_mc2_blink` y `cursada_mc2_app`
- flash con OpenOCD
- sesion de GDB detenida en `main`

Si el toolchain no esta en `PATH`, podes configurar una ruta explicita al momento de configurar. Usa la carpeta raiz del toolchain o su subcarpeta `bin`, segun tu instalacion:

```powershell
cmake --preset debug -DARM_NONE_EABI_TOOLCHAIN_PATH="<ruta-al-toolchain-o-bin>"
```

## Targets disponibles

- `cursada_mc2_blink`: firmware minimo para validar build, flash y debug.
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
cmake --build --preset debug --target cursada_mc2_blink
cmake --build --preset debug --target cursada_mc2_app
```

Release:

```powershell
cmake --preset release
cmake --build --preset release --target cursada_mc2_blink
cmake --build --preset release --target cursada_mc2_app
```

## Flash desde terminal

Blink:

```powershell
cmake --build --preset debug --target flash_cursada_mc2_blink
```

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
2. Ejecutar `Build Blink [debug]` o `Build App [debug]`.
3. Elegir `Debug Blink (OpenOCD)` o `Debug App (OpenOCD)` en la pestana Run and Debug.
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

1. Compilar `cursada_mc2_blink`.
2. Verificar `build/debug/cursada_mc2_blink.elf`, `.bin` y `.hex`.
3. Flashear `cursada_mc2_blink`.
4. Abrir `Debug Blink (OpenOCD)` en VS Code.
5. Confirmar que el debugger se detiene en `main`.
6. Repetir el flujo con `cursada_mc2_app`.

## Nota sobre el arbol `Debug/`

El directorio `Debug/` heredado de MCUXpresso quedo como referencia historica. El flujo reproducible actual usa exclusivamente `build/debug` y `build/release`.
