# startup

Contiene el arranque bare-metal especifico del core M4 del LPC4337.

Archivos:
- `cr_startup_lpc43xx.c`: vector table y arranque temprano
- `sysinit.c`: `SystemInit` y configuracion inicial del chip
- `crp.c`: palabra de Code Read Protect heredada de la base NXP

Estos archivos provienen de una base NXP/MCUXpresso-LPCOpen y se conservan por compatibilidad con la placa y el linker actual.
