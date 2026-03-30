/**
 * @file control_on_off.c
 * @brief Implementacion de control on/off con histeresis.
 */

#include "control/control_on_off.h"

#include <limits.h>

typedef struct {
    control_on_off_configuracion_t configuracion;
    uint32_t tiempo_en_estado_ms;
    bool salida_activa;
    bool tiene_medicion;
    int16_t ultima_medicion_deci_celsius;
} control_on_off_estado_t;

static control_on_off_estado_t control_on_off_ = {0};

void control_on_off_inicializar(control_on_off_configuracion_t configuracion)
{
    control_on_off_.configuracion = configuracion;
    control_on_off_.tiempo_en_estado_ms = 0U;
    control_on_off_.salida_activa = false;
    control_on_off_.tiene_medicion = false;
    control_on_off_.ultima_medicion_deci_celsius = 0;
}

void control_on_off_configurar(control_on_off_configuracion_t configuracion)
{
    control_on_off_.configuracion = configuracion;
}

control_on_off_configuracion_t control_on_off_obtener_configuracion(void)
{
    return control_on_off_.configuracion;
}

void control_on_off_reiniciar(void)
{
    control_on_off_.tiempo_en_estado_ms = 0U;
    control_on_off_.salida_activa = false;
    control_on_off_.tiene_medicion = false;
    control_on_off_.ultima_medicion_deci_celsius = 0;
}

void control_on_off_procesar(int16_t medicion, uint32_t delta_tiempo_ms)
{
    bool salida_deseada = false;
    int16_t umbral_activacion = 0;
    int16_t umbral_corte = 0;
    uint32_t tiempo_minimo_requerido_ms = 0U;

    if (UINT_MAX - control_on_off_.tiempo_en_estado_ms < delta_tiempo_ms) {
        control_on_off_.tiempo_en_estado_ms = UINT_MAX;
    } else {
        control_on_off_.tiempo_en_estado_ms += delta_tiempo_ms;
    }

    control_on_off_.ultima_medicion_deci_celsius = medicion;
    control_on_off_.tiene_medicion = true;

    if (!control_on_off_.configuracion.habilitado) {
        control_on_off_.salida_activa = false;
        control_on_off_.tiempo_en_estado_ms = 0U;
        return;
    }

    umbral_corte = control_on_off_.configuracion.setpoint_deci_celsius;
    if (control_on_off_.configuracion.sentido == CONTROL_ON_OFF_SENTIDO_CALENTAR) {
        umbral_activacion = (int16_t) (control_on_off_.configuracion.setpoint_deci_celsius
                                       - (int16_t) control_on_off_.configuracion.histeresis_deci_celsius);
        if (medicion <= umbral_activacion) {
            salida_deseada = true;
        } else if (medicion >= umbral_corte) {
            salida_deseada = false;
        } else {
            salida_deseada = control_on_off_.salida_activa;
        }
    } else {
        umbral_activacion = (int16_t) (control_on_off_.configuracion.setpoint_deci_celsius
                                       + (int16_t) control_on_off_.configuracion.histeresis_deci_celsius);
        if (medicion >= umbral_activacion) {
            salida_deseada = true;
        } else if (medicion <= umbral_corte) {
            salida_deseada = false;
        } else {
            salida_deseada = control_on_off_.salida_activa;
        }
    }

    if (salida_deseada == control_on_off_.salida_activa) {
        return;
    }

    if (salida_deseada) {
        tiempo_minimo_requerido_ms = control_on_off_.configuracion.tiempo_minimo_apagado_ms;
    } else {
        tiempo_minimo_requerido_ms = control_on_off_.configuracion.tiempo_minimo_encendido_ms;
    }

    if (control_on_off_.tiempo_en_estado_ms < tiempo_minimo_requerido_ms) {
        return;
    }

    control_on_off_.salida_activa = salida_deseada;
    control_on_off_.tiempo_en_estado_ms = 0U;
}

bool control_on_off_esta_salida_activa(void)
{
    return control_on_off_.salida_activa;
}

bool control_on_off_tiene_medicion(void)
{
    return control_on_off_.tiene_medicion;
}

int16_t control_on_off_obtener_ultima_medicion(void)
{
    return control_on_off_.ultima_medicion_deci_celsius;
}
