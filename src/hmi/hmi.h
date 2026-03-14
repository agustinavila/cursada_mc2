/**
 * @file hmi.h
 * @brief Interfaz de la HMI jerarquica para LCD y pulsadores de la EDU-CIAA
 */

#if !defined(HMI_H_)
#define HMI_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa el estado interno de la HMI y dibuja la pantalla inicial.
 *
 * Debe llamarse una sola vez luego de haber inicializado los drivers de
 * hardware necesarios, en particular el LCD y la capa de delays.
 */
void hmi_init(void);

/**
 * @brief Procesa la navegacion de la interfaz y actualiza el LCD si es necesario.
 *
 * Esta funcion debe llamarse de manera periodica desde el lazo principal.
 * Internamente:
 * - lee el estado de los pulsadores,
 * - detecta eventos de navegacion,
 * - actualiza el estado de la HMI,
 * - y redibuja la pantalla cuando corresponde.
 */
void hmi_process(void);

/**
 * @brief Obtiene la temperatura cacheada de un sensor especifico.
 *
 * Esta funcion desacopla la logica de control de la rotacion visual de la HMI.
 * Permite fijar un sensor concreto como entrada de proceso, independientemente
 * de cual se este mostrando en la pantalla principal.
 *
 * @param indice_sensor Indice del sensor dentro de la tabla descubierta.
 * @param temperatura_deci_celsius Variable de salida.
 *
 * @retval true Si hay una medicion valida para ese sensor.
 * @retval false Si el indice es invalido, no hay muestra valida o el puntero es invalido.
 */
bool hmi_obtener_temperatura_sensor(uint8_t indice_sensor, int16_t* temperatura_deci_celsius);

/**
 * @brief Carga en la HMI los parametros de control vigentes.
 *
 * @param setpoint_deci_celsius Setpoint en decimas de grado Celsius.
 * @param histeresis_deci_celsius Histeresis en decimas de grado Celsius.
 * @param modo_calentar `true` si el modo es calentar.
 */
void hmi_cargar_parametros_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   bool modo_calentar);

/**
 * @brief Obtiene el setpoint configurado desde la HMI.
 *
 * El valor se devuelve en decimas de grado Celsius para que la aplicacion
 * pueda usarlo directamente en la capa de control.
 *
 * @return Setpoint actual en decimas de grado Celsius.
 */
int16_t hmi_obtener_setpoint_deci_celsius(void);

/**
 * @brief Obtiene la histeresis configurada desde la HMI.
 *
 * El valor se devuelve en decimas de grado Celsius para que la aplicacion
 * pueda usarlo directamente en la capa de control.
 *
 * @return Histeresis actual en decimas de grado Celsius.
 */
uint16_t hmi_obtener_histeresis_deci_celsius(void);

/**
 * @brief Indica si el modo configurado en la HMI corresponde a calentar.
 *
 * @retval true Si el modo actual es calentar.
 * @retval false Si el modo actual es enfriar.
 */
bool hmi_modo_control_es_calentar(void);

/**
 * @brief Consume la solicitud de restablecer parametros a defaults.
 *
 * @retval true Si la HMI solicito restablecer valores.
 * @retval false Si no hay solicitud pendiente.
 */
bool hmi_consumir_solicitud_restablecer_parametros(void);

#endif // HMI_H_
