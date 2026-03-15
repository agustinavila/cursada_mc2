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
 * @brief Carga en la HMI el estado actual del sensor de proceso.
 *
 * La aplicacion es la dueña del bus DS18B20 y le entrega a la HMI solamente
 * el valor visible del sensor usado por el control.
 *
 * @param temperatura_valida `true` si la temperatura actual es valida.
 * @param temperatura_deci_celsius Temperatura en decimas de grado Celsius.
 */
void hmi_cargar_estado_sensor(bool temperatura_valida, int16_t temperatura_deci_celsius);

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
 * @brief Carga en la HMI el estado actual del lazo de control.
 *
 * @param salida_activa `true` si la salida del control esta activa.
 * @param sensor_resuelto `true` si hay un sensor de proceso valido asignado.
 * @param indice_sensor_proceso Indice del sensor de proceso actualmente usado.
 */
void hmi_cargar_estado_control(bool salida_activa,
                               bool sensor_resuelto,
                               uint8_t indice_sensor_proceso);

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

#endif // HMI_H_
