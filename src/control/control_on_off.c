/**
 * @file control_on_off.c
 * @brief Implementacion de control on/off con histeresis.
 */

#include "control/control_on_off.h"

static bool control_on_off_configuracion_es_valida(const control_on_off_configuracion_t* configuracion)
{
    if (configuracion == 0) {
        return false;
    }

    if ((configuracion->sentido != CONTROL_ON_OFF_SENTIDO_CALENTAR)
        && (configuracion->sentido != CONTROL_ON_OFF_SENTIDO_ENFRIAR)) {
        return false;
    }

    return true;
}

static int16_t control_on_off_obtener_umbral_inferior(const control_on_off_t* control)
{
    return (int16_t) (control->configuracion.setpoint_deci_celsius
                      - (int16_t) (control->configuracion.histeresis_deci_celsius / 2U));
}

static int16_t control_on_off_obtener_umbral_superior(const control_on_off_t* control)
{
    return (int16_t) (control->configuracion.setpoint_deci_celsius
                      + (int16_t) (control->configuracion.histeresis_deci_celsius / 2U));
}

static void control_on_off_reiniciar_impl(control_generico_t* base)
{
    control_on_off_t* control = (control_on_off_t*) base;

    if ((control == 0) || !control->inicializado) {
        return;
    }

    control->salida_activa = false;
    control->tiene_medicion = false;
    control->ultima_medicion_deci_celsius = 0;
}

static bool control_on_off_procesar_impl(control_generico_t* base, int16_t medicion)
{
    control_on_off_t* control = (control_on_off_t*) base;

    if ((control == 0) || !control->inicializado) {
        return false;
    }

    control->ultima_medicion_deci_celsius = medicion;
    control->tiene_medicion = true;

    if (!control->configuracion.habilitado) {
        control->salida_activa = false;
        return true;
    }

    if (control->configuracion.sentido == CONTROL_ON_OFF_SENTIDO_CALENTAR) {
        const int16_t umbral_inferior = control_on_off_obtener_umbral_inferior(control);
        const int16_t umbral_superior = control_on_off_obtener_umbral_superior(control);

        if (medicion <= umbral_inferior) {
            control->salida_activa = true;
        } else if (medicion >= umbral_superior) {
            control->salida_activa = false;
        }
    } else {
        const int16_t umbral_inferior = control_on_off_obtener_umbral_inferior(control);
        const int16_t umbral_superior = control_on_off_obtener_umbral_superior(control);

        if (medicion >= umbral_superior) {
            control->salida_activa = true;
        } else if (medicion <= umbral_inferior) {
            control->salida_activa = false;
        }
    }

    return true;
}

static bool control_on_off_esta_salida_activa_impl(const control_generico_t* base)
{
    const control_on_off_t* control = (const control_on_off_t*) base;

    if ((control == 0) || !control->inicializado) {
        return false;
    }

    return control->salida_activa;
}

static bool control_on_off_obtener_ultima_medicion_impl(const control_generico_t* base, int16_t* medicion)
{
    const control_on_off_t* control = (const control_on_off_t*) base;

    if ((control == 0) || (medicion == 0) || !control->inicializado || !control->tiene_medicion) {
        return false;
    }

    *medicion = control->ultima_medicion_deci_celsius;
    return true;
}

static const control_operaciones_t control_on_off_operaciones_ = {
    .reiniciar = control_on_off_reiniciar_impl,
    .procesar = control_on_off_procesar_impl,
    .salida_activa = control_on_off_esta_salida_activa_impl,
    .obtener_ultima_medicion = control_on_off_obtener_ultima_medicion_impl,
};

bool control_on_off_inicializar(control_on_off_t* control,
                                const control_on_off_configuracion_t* configuracion)
{
    if ((control == 0) || !control_on_off_configuracion_es_valida(configuracion)) {
        return false;
    }

    control->base.operaciones = &control_on_off_operaciones_;
    control->configuracion = *configuracion;
    control->salida_activa = false;
    control->inicializado = true;
    control->tiene_medicion = false;
    control->ultima_medicion_deci_celsius = 0;
    return true;
}

bool control_on_off_configurar(control_on_off_t* control,
                               const control_on_off_configuracion_t* configuracion)
{
    if ((control == 0) || !control->inicializado || !control_on_off_configuracion_es_valida(configuracion)) {
        return false;
    }

    control->configuracion = *configuracion;
    control->salida_activa = false;
    control->tiene_medicion = false;
    return true;
}

bool control_on_off_obtener_configuracion(const control_on_off_t* control,
                                          control_on_off_configuracion_t* configuracion)
{
    if ((control == 0) || (configuracion == 0) || !control->inicializado) {
        return false;
    }

    *configuracion = control->configuracion;
    return true;
}

control_generico_t* control_on_off_como_generico(control_on_off_t* control)
{
    if (control == 0) {
        return 0;
    }

    return &control->base;
}

const control_generico_t* control_on_off_como_generico_const(const control_on_off_t* control)
{
    if (control == 0) {
        return 0;
    }

    return &control->base;
}
