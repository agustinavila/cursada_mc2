/**
 * @file control_on_off.h
 * @brief Implementacion de control on/off con histeresis.
 */

#if !defined(CONTROL_CONTROL_ON_OFF_H_)
#define CONTROL_CONTROL_ON_OFF_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sentido de accion del control on/off.
 *
 * - CALENTAR: activa la salida cuando la temperatura esta por debajo del objetivo.
 * - ENFRIAR: activa la salida cuando la temperatura esta por encima del objetivo.
 */
typedef enum {
    CONTROL_ON_OFF_SENTIDO_CALENTAR = 0,
    CONTROL_ON_OFF_SENTIDO_ENFRIAR,
} control_on_off_sentido_t;

/**
 * @brief Configuracion del control on/off con histeresis.
 *
 * Todas las temperaturas se expresan en decimas de grado Celsius para evitar
 * el uso de punto flotante dentro del lazo de control.
 */
typedef struct {
    control_on_off_sentido_t sentido;
    int16_t setpoint_deci_celsius;      // setpoint en decimas de grado Celsius
    uint16_t histeresis_deci_celsius;   // histeresis en decimas de grado Celsius
    uint32_t tiempo_minimo_encendido_ms; // permanencia minima en estado encendido
    uint32_t tiempo_minimo_apagado_ms;   // permanencia minima en estado apagado
    bool habilitado;                     // habilita o deshabilita el control
} control_on_off_configuracion_t;

/**
 * @brief Inicializa el control on/off.
 *
 * @param configuracion Configuracion inicial.
 */
void control_on_off_inicializar(control_on_off_configuracion_t configuracion);

/**
 * @brief Actualiza la configuracion del control on/off.
 *
 * @param configuracion Nueva configuracion.
 */
void control_on_off_configurar(control_on_off_configuracion_t configuracion);

/**
 * @brief Obtiene la configuracion actual.
 */
control_on_off_configuracion_t control_on_off_obtener_configuracion(void);

/**
 * @brief Restablece el estado dinamico del controlador.
 */
void control_on_off_reiniciar(void);

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

/**
 * @brief Indica si ya se proceso al menos una medicion.
 */
bool control_on_off_tiene_medicion(void);

/**
 * @brief Obtiene la ultima medicion procesada.
 *
 * @return Ultima medicion en decimas de grado Celsius.
 */
int16_t control_on_off_obtener_ultima_medicion(void);

#endif // CONTROL_CONTROL_ON_OFF_H_
