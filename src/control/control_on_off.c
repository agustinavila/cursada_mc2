/**
 * @file control_on_off.c
 * @brief Implementacion de control on/off con histeresis.
 */

#include "control/control_on_off.h"

#include <limits.h>

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

static int16_t control_on_off_obtener_umbral_activacion(const control_on_off_t* control)
{
    if (control->configuracion.sentido == CONTROL_ON_OFF_SENTIDO_CALENTAR) {
        return (int16_t) (control->configuracion.setpoint_deci_celsius
                          - (int16_t) control->configuracion.histeresis_deci_celsius);
    }

    return (int16_t) (control->configuracion.setpoint_deci_celsius
                      + (int16_t) control->configuracion.histeresis_deci_celsius);
}

static int16_t control_on_off_obtener_umbral_corte(const control_on_off_t* control)
{
    return control->configuracion.setpoint_deci_celsius;
}

static bool control_on_off_salida_deseada(const control_on_off_t* control, int16_t medicion)
{
    const int16_t umbral_activacion = control_on_off_obtener_umbral_activacion(control);
    const int16_t umbral_corte = control_on_off_obtener_umbral_corte(control);

    if (control->configuracion.sentido == CONTROL_ON_OFF_SENTIDO_CALENTAR) {
        if (medicion <= umbral_activacion) {
            return true;
        }
        if (medicion >= umbral_corte) {
            return false;
        }
    } else {
        if (medicion >= umbral_activacion) {
            return true;
        }
        if (medicion <= umbral_corte) {
            return false;
        }
    }

    return control->salida_activa;
}

static uint32_t control_on_off_obtener_tiempo_minimo_para_conmutar(const control_on_off_t* control,
                                                                   bool nueva_salida_activa)
{
    if (nueva_salida_activa) {
        return control->configuracion.tiempo_minimo_apagado_ms;
    }

    return control->configuracion.tiempo_minimo_encendido_ms;
}

static bool control_on_off_puede_conmutar(const control_on_off_t* control, bool nueva_salida_activa)
{
    return control->tiempo_en_estado_ms
        >= control_on_off_obtener_tiempo_minimo_para_conmutar(control, nueva_salida_activa);
}

bool control_on_off_inicializar(control_on_off_t* control,
                                const control_on_off_configuracion_t* configuracion)
{
    if ((control == 0) || !control_on_off_configuracion_es_valida(configuracion)) {
        return false;
    }

    control->configuracion = *configuracion;
    control->tiempo_en_estado_ms = 0U;
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
    control->tiempo_en_estado_ms = 0U;
    control->salida_activa = false;
    control->tiene_medicion = false;
    control->ultima_medicion_deci_celsius = 0;
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

void control_on_off_reiniciar(control_on_off_t* control)
{
    if ((control == 0) || !control->inicializado) {
        return;
    }

    control->tiempo_en_estado_ms = 0U;
    control->salida_activa = false;
    control->tiene_medicion = false;
    control->ultima_medicion_deci_celsius = 0;
}

bool control_on_off_procesar(control_on_off_t* control, int16_t medicion, uint32_t delta_tiempo_ms)
{
    bool salida_deseada = false;

    if ((control == 0) || !control->inicializado) {
        return false;
    }

    if (UINT_MAX - control->tiempo_en_estado_ms < delta_tiempo_ms) {
        control->tiempo_en_estado_ms = UINT_MAX;
    } else {
        control->tiempo_en_estado_ms += delta_tiempo_ms;
    }

    control->ultima_medicion_deci_celsius = medicion;
    control->tiene_medicion = true;

    if (!control->configuracion.habilitado) {
        control->salida_activa = false;
        control->tiempo_en_estado_ms = 0U;
        return true;
    }

    salida_deseada = control_on_off_salida_deseada(control, medicion);
    if (salida_deseada == control->salida_activa) {
        return true;
    }

    if (!control_on_off_puede_conmutar(control, salida_deseada)) {
        return true;
    }

    control->salida_activa = salida_deseada;
    control->tiempo_en_estado_ms = 0U;
    return true;
}

bool control_on_off_esta_salida_activa(const control_on_off_t* control)
{
    if ((control == 0) || !control->inicializado) {
        return false;
    }

    return control->salida_activa;
}

bool control_on_off_obtener_ultima_medicion(const control_on_off_t* control, int16_t* medicion)
{
    if ((control == 0) || (medicion == 0) || !control->inicializado || !control->tiene_medicion) {
        return false;
    }

    *medicion = control->ultima_medicion_deci_celsius;
    return true;
}
