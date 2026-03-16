# drivers

Contiene los drivers propios del proyecto.

Responsabilidades actuales:
- acceso a GPIO, timers, UART, LCD y pulsadores
- soporte 1-Wire y DS18B20
- EEPROM interna para persistencia
- drivers auxiliares del board support propio

Regla practica:
- el codigo especifico del proyecto vive aca
- el acceso de mas bajo nivel al LPC4337 sigue viniendo de `third_party/lpcopen/chip_43xx`
