# Tests host-side

Esta carpeta contiene la primera etapa de tests nativos del repo.

- `tests/control/`: pruebas de la logica de control desacoplada de hardware.
- `tests/app/`: pruebas de `parametros` y defaults persistentes.
- `tests/support/`: harness minimo y dobles de prueba usados solo por los tests.

En esta etapa:

- los tests se ejecutan en host con `CTest`
- la cobertura usa `gcov` solo sobre binarios host
- no se mezclan con el firmware ARM, OpenOCD ni el linker bare-metal
