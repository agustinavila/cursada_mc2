/**
 * @file hmi.h
 * @brief Interfaz publica de la HMI.
 */

#if !defined(HMI_H_)
#define HMI_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t setpoint_deci_celsius;
    uint16_t histeresis_deci_celsius;
    uint32_t tiempo_minimo_encendido_ms;
    uint32_t tiempo_minimo_apagado_ms;
    bool modo_calentar;
} hmi_parametros_control_t;

typedef struct {
    bool temperatura_valida;
    int16_t temperatura_deci_celsius;
    bool salida_activa;
    bool sensor_disponible;
} hmi_estado_proceso_t;

/**
 * @brief Inicializa el estado interno de la HMI y dibuja la pantalla inicial.
 *
 * Debe llamarse una sola vez luego de haber inicializado los drivers de
 * hardware necesarios, en particular el LCD.
 */
void hmi_init(void);

/**
 * @brief Procesa la navegacion de la interfaz y actualiza el LCD si es necesario.
 *
 * Esta funcion debe llamarse de manera periodica desde el lazo principal.
 */
void hmi_process(void);

/**
 * @brief Carga en la HMI los parametros de control vigentes.
 *
 * @param parametros Estructura con los parametros visibles/editables del control.
 */
void hmi_cargar_parametros_control(const hmi_parametros_control_t* parametros);

/**
 * @brief Carga en la HMI el estado visible actual del proceso.
 *
 * @param estado Estructura con temperatura, presencia de sensor y estado de salida.
 */
void hmi_cargar_estado_proceso(const hmi_estado_proceso_t* estado);

/**
 * @brief Obtiene todos los parametros de control actualmente cargados en la HMI.
 *
 * @return Copia de los parametros editables actuales.
 */
hmi_parametros_control_t hmi_obtener_parametros_control(void);

#endif // HMI_H_
