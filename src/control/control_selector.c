/**
 * @file control_selector.c
 * @brief Implementacion del selector de estrategias de control.
 */

#include "control/control_selector.h"

bool control_selector_inicializar(control_selector_t* selector)
{
    if (selector == 0) {
        return false;
    }

    selector->tipo_activo = CONTROL_TIPO_NINGUNO;
    selector->control_activo = 0;
    return true;
}

void control_selector_deshabilitar(control_selector_t* selector)
{
    if (selector == 0) {
        return;
    }

    selector->tipo_activo = CONTROL_TIPO_NINGUNO;
    selector->control_activo = 0;
}

bool control_selector_seleccionar_on_off(control_selector_t* selector,
                                         const control_on_off_configuracion_t* configuracion)
{
    if ((selector == 0) || (configuracion == 0)) {
        return false;
    }

    if (!control_on_off_inicializar(&selector->instancia_on_off, configuracion)) {
        return false;
    }

    selector->tipo_activo = CONTROL_TIPO_ON_OFF;
    selector->control_activo = control_on_off_como_generico(&selector->instancia_on_off);
    return true;
}

control_tipo_t control_selector_obtener_tipo(const control_selector_t* selector)
{
    if (selector == 0) {
        return CONTROL_TIPO_NINGUNO;
    }

    return selector->tipo_activo;
}

control_generico_t* control_selector_obtener_activo(const control_selector_t* selector)
{
    if (selector == 0) {
        return 0;
    }

    return selector->control_activo;
}

const control_generico_t* control_selector_obtener_activo_const(const control_selector_t* selector)
{
    if (selector == 0) {
        return 0;
    }

    return selector->control_activo;
}
