/**
 * @file parametros_default.c
 * @brief Valores por defecto de los parametros persistentes.
 */

#include "app/parametros_default.h"

/**
 * @brief Valores iniciales usados tanto en primer arranque como en reset.
 */
static const parametros_t parametros_default_ = {
    .control = {
        .setpoint_deci_celsius = 270,
        .histeresis_deci_celsius = 20U,
        .tiempo_minimo_encendido_ms = 0U,
        .tiempo_minimo_apagado_ms = 0U,
        .modo_calentar = true,
    },
};

/**
 * @brief Devuelve la instancia constante con los defaults de la aplicacion.
 */
const parametros_t* parametros_default_obtener(void)
{
    return &parametros_default_;
}
