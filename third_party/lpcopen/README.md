# lpcopen

Esta carpeta contiene codigo vendor heredado de LPCOpen.

Uso en este repo:
- se usa solo `chip_43xx` como capa de soporte del LPC4337
- no se usa board library
- no se deben editar archivos internos salvo un motivo muy justificado

Integracion actual:
- CMake compila solo el subconjunto de fuentes necesario para el firmware
- los warnings del vendor se silencian para no contaminar la salida del proyecto propio
