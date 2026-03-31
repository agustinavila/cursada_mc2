/**
 * @file control_on_off.h
 * @brief Implementacion de control on/off con histeresis.
 */

#if !defined(CONTROL_CONTROL_ON_OFF_H_)
#define CONTROL_CONTROL_ON_OFF_H_

#include "app/parametros.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa el control on/off.
 *
 * @param parametros Parametros iniciales del control.
 */
void control_on_off_inicializar(parametros_control_t parametros);

/**
 * @brief Actualiza la configuracion del control on/off.
 *
 * @param parametros Nuevos parametros del control.
 */
void control_on_off_configurar(parametros_control_t parametros);

/**
 * @brief Procesa una nueva medicion y actualiza la salida del control.
 *
 * @param medicion Medicion actual del proceso.
 * @param delta_tiempo_ms Tiempo transcurrido desde la ultima llamada.
 */
void control_on_off_procesar(int16_t medicion, uint32_t delta_tiempo_ms);

/**
 * @brief Indica si la salida del control esta activa.
 */
bool control_on_off_esta_salida_activa(void);

#endif // CONTROL_CONTROL_ON_OFF_H_
