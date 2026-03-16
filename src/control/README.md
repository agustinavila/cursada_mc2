# control

Contiene las estrategias de control y la capa comun que las abstrae.

Responsabilidades actuales:
- API comun de control
- implementacion `on/off` con histeresis
- seleccion de estrategia activa

Archivos relevantes:
- `control.c`: interfaz comun
- `control_on_off.c`: estrategia actual
- `control_selector.c`: seleccion del controlador
