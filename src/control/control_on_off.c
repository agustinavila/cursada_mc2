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
    uint32_t tiempo_minimo_requerido_ms = 0U;

    if (UINT_MAX - control_on_off_.tiempo_en_estado_ms < delta_tiempo_ms) {
        control_on_off_.tiempo_en_estado_ms = UINT_MAX;
    } else {
        control_on_off_.tiempo_en_estado_ms += delta_tiempo_ms;
    }

    if (control_on_off_.parametros.modo_calentar) {
        const int16_t umbral_corte = control_on_off_.parametros.setpoint_deci_celsius;
        // Para calentar, este es el umbral inferior donde la salida vuelve a activarse.
        const int16_t umbral_activacion = (int16_t) (control_on_off_.parametros.setpoint_deci_celsius
                                                     - (int16_t) control_on_off_.parametros.histeresis_deci_celsius);

        if (medicion <= umbral_activacion) {
            salida_deseada = true;
        } else if (medicion >= umbral_corte) {
            salida_deseada = false;
        } else {
            salida_deseada = control_on_off_.salida_activa;
        }
    } else {
        const int16_t umbral_corte = control_on_off_.parametros.setpoint_deci_celsius;
        // Para enfriar, este es el umbral superior donde la salida vuelve a activarse.
        const int16_t umbral_activacion = (int16_t) (control_on_off_.parametros.setpoint_deci_celsius
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
        // Para pasar a encendido, se respeta el tiempo minimo en apagado.
        tiempo_minimo_requerido_ms = control_on_off_.parametros.tiempo_minimo_apagado_ms;
    } else {
        // Para pasar a apagado, se respeta el tiempo minimo en encendido.
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
