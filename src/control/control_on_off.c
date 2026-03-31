/**
 * @file control_on_off.c
 * @brief Implementacion de control on/off con histeresis.
 */

#include "control/control_on_off.h"

#include <limits.h>

typedef struct {
    parametros_control_t parametros;
    uint32_t tiempo_en_estado_ms;
    bool salida_activa;
} control_on_off_estado_t;

static control_on_off_estado_t control_on_off_ = {0};

void control_on_off_inicializar(parametros_control_t parametros)
{
    control_on_off_.parametros = parametros;
    control_on_off_.tiempo_en_estado_ms = 0U;
    control_on_off_.salida_activa = false;
}

void control_on_off_configurar(parametros_control_t parametros)
{
    control_on_off_.parametros = parametros;
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

    umbral_corte = control_on_off_.parametros.setpoint_deci_celsius;
    if (control_on_off_.parametros.modo_calentar) {
        // Para calentar se activa por debajo del setpoint menos histeresis.
        umbral_activacion = (int16_t) (control_on_off_.parametros.setpoint_deci_celsius
                                       - (int16_t) control_on_off_.parametros.histeresis_deci_celsius);
        if (medicion <= umbral_activacion) {
            salida_deseada = true;
        } else if (medicion >= umbral_corte) {
            salida_deseada = false;
        } else {
            salida_deseada = control_on_off_.salida_activa;
        }
    } else {
        // Para enfriar se activa por encima del setpoint mas histeresis.
        umbral_activacion = (int16_t) (control_on_off_.parametros.setpoint_deci_celsius
                                       + (int16_t) control_on_off_.parametros.histeresis_deci_celsius);
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
        tiempo_minimo_requerido_ms = control_on_off_.parametros.tiempo_minimo_apagado_ms;
    } else {
        tiempo_minimo_requerido_ms = control_on_off_.parametros.tiempo_minimo_encendido_ms;
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
