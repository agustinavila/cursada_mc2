# cursada_mc2

Entorno reproducible para la EDU-CIAA-NXP (LPC4337, core M4) en Windows usando VS Code, CMake, OpenOCD y GDB, sin PlatformIO ni Eclipse.

## Estructura

- `cmake/`: toolchain y helpers de CMake para bare-metal.
- `platform/lpc43xx/`: código vendor heredado de NXP/LPCOpen que se conserva como capa de chip support.
- `platform/openocd/ciaa-nxp.cfg`: configuración de OpenOCD validada para la interfaz FTDI/JTAG de la EDU-CIAA-NXP.
- `src/`: startup, system init, aplicación actual y target mínimo de validación.
- `.vscode/`: tasks, launch y settings para VS Code.

## Prerrequisitos en Windows

Tienen que estar accesibles desde `PATH`:

- `cmake`
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `openocd`

Verificación rápida:

```powershell
cmake --version
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
openocd --version
```

Si el toolchain no está en `PATH`, podés configurar una ruta explícita al momento de configurar:

```powershell
cmake --preset debug -DARM_NONE_EABI_TOOLCHAIN_PATH="C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1"
```

## Targets disponibles

- `cursada_mc2_blink`: firmware mínimo para validar build, flash y debug.
- `cursada_mc2_app`: aplicación actual con los drivers existentes.

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

Aplicación actual:

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
3. Elegir `Debug Blink (OpenOCD)` o `Debug App (OpenOCD)` en la pestaña Run and Debug.
4. El debugger usa `runToEntryPoint: main`, así que debe detenerse en `main`.

## Validación de OpenOCD y del probe FTDI

Validación mínima de conectividad:

```powershell
openocd -f platform/openocd/ciaa-nxp.cfg -c "init; targets; shutdown"
```

Ese comando debería detectar al menos el TAP `lpc4337.m4`.

Para inspeccionar dispositivos USB/driver en Windows:

```powershell
Get-PnpDevice -PresentOnly | Where-Object { $_.FriendlyName -match 'FTDI|CMSIS|J-Link|ST-Link|LPC-Link|NXP|CIAA' } | Format-Table -Auto
```

Si necesitás ver más detalle del FTDI:

```powershell
pnputil /enum-devices /connected
```

Buscar la interfaz FTDI con `VID 0403` y `PID 6010`.

## Rol de LPCOpen en esta base

LPCOpen queda reducido a vendor code de soporte:

- CMSIS headers
- chip support
- startup/system integration alineada con LPC4337

No se usa board library. El código propio vive en `src/` y se apoya en `lpc_chip_43xx` como capa de bajo nivel.

## Troubleshooting

### OpenOCD no detecta la placa

- Verificar que la placa esté alimentada y conectada.
- Probar `openocd -f platform/openocd/ciaa-nxp.cfg -c "init; targets; shutdown"`.
- Verificar si Windows está exponiendo la interfaz FTDI correcta.
- No cambiar drivers si OpenOCD ya detecta el TAP.
- Usar Zadig y `WinUSB` sólo si OpenOCD deja de abrir la interfaz FTDI o no aparece el dispositivo correcto.

### GDB no conecta

- Confirmar que `arm-none-eabi-gdb` esté en `PATH`.
- Confirmar que `openocd` arranca sin errores con el mismo `cfg`.
- Revisar que el `launch.json` apunte al `.elf` del preset `debug`.

### VS Code no encuentra el toolchain

- Verificar `arm-none-eabi-gcc --version`.
- Si no está en `PATH`, configurar `ARM_NONE_EABI_TOOLCHAIN_PATH` al correr `cmake --preset debug`.
- Reconfigurar el preset después del cambio.

### Problemas con rutas en Windows

- Esta base usa `build/debug` y `build/release`, no el árbol legado `Debug/`.
- Si movés el repo de carpeta, reconfigurá con `cmake --preset debug`.
- No reutilices `CMakeCache.txt` viejos de otra ruta.

### Errores de linker o startup

- Confirmar que el target sea el core M4.
- Confirmar que no se haya cambiado el linker script base bajo `platform/lpc43xx/ldscripts/default/`.
- Confirmar que `src/cr_startup_lpc43xx.c` y `src/sysinit.c` estén siendo compilados.

### Problemas por dependencias viejas de LPCOpen

- Evitar agregar código nuevo que dependa del board library.
- Mantener el acceso a periféricos a través de drivers propios o directamente sobre `chip.h`.
- Si aparece una dependencia a símbolos de MCUXpresso/Code Red, revisá primero si viene de headers viejos o includes heredados.

## Validación mínima sugerida

1. Compilar `cursada_mc2_blink`.
2. Verificar `build/debug/cursada_mc2_blink.elf`, `.bin` y `.hex`.
3. Flashear `cursada_mc2_blink`.
4. Abrir `Debug Blink (OpenOCD)` en VS Code.
5. Confirmar que el debugger se detiene en `main`.
6. Repetir el flujo con `cursada_mc2_app`.

## Nota sobre el árbol `Debug/`

El directorio `Debug/` heredado de MCUXpresso quedó como referencia histórica. El flujo reproducible actual usa exclusivamente `build/debug` y `build/release`.
