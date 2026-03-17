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
 * @brief Instancia concreta del control on/off.
 */
typedef struct {
    control_on_off_configuracion_t configuracion; // configuracion vigente del controlador
    uint32_t tiempo_en_estado_ms;                // tiempo acumulado desde el ultimo cambio de salida
    bool salida_activa;                          // salida actual del lazo on/off
    bool inicializado;                           // indica si la instancia fue inicializada
    bool tiene_medicion;                         // indica si ya se proceso al menos una medicion
    int16_t ultima_medicion_deci_celsius;        // ultima medicion procesada en decimas de grado Celsius
} control_on_off_t;

/**
 * @brief Inicializa una instancia de control on/off.
 *
 * @param control Instancia concreta.
 * @param configuracion Configuracion inicial.
 *
 * @retval true Si la inicializacion fue correcta.
 * @retval false Si algun puntero es invalido o la configuracion no es valida.
 */
bool control_on_off_inicializar(control_on_off_t* control,
                                const control_on_off_configuracion_t* configuracion);

/**
 * @brief Actualiza la configuracion del control on/off.
 *
 * @param control Instancia concreta.
 * @param configuracion Nueva configuracion.
 *
 * @retval true Si la configuracion fue aceptada.
 * @retval false Si algun puntero es invalido o la configuracion no es valida.
 */
bool control_on_off_configurar(control_on_off_t* control,
                               const control_on_off_configuracion_t* configuracion);

/**
 * @brief Obtiene la configuracion actual.
 *
 * @param control Instancia concreta.
 * @param configuracion Estructura de salida.
 *
 * @retval true Si la operacion fue correcta.
 * @retval false Si algun puntero es invalido o el control no esta inicializado.
 */
bool control_on_off_obtener_configuracion(const control_on_off_t* control,
                                          control_on_off_configuracion_t* configuracion);

/**
 * @brief Restablece el estado dinamico del controlador.
 *
 * @param control Instancia concreta.
 */
void control_on_off_reiniciar(control_on_off_t* control);

/**
 * @brief Procesa una nueva medicion y actualiza la salida del control.
 *
 * @param control Instancia concreta.
 * @param medicion Medicion actual del proceso.
 * @param delta_tiempo_ms Tiempo transcurrido desde la ultima llamada.
 *
 * @retval true Si el controlador pudo procesar la medicion.
 * @retval false Si la instancia no es valida o no esta inicializada.
 */
bool control_on_off_procesar(control_on_off_t* control, int16_t medicion, uint32_t delta_tiempo_ms);

/**
 * @brief Indica si la salida del control esta activa.
 *
 * @param control Instancia concreta.
 *
 * @retval true Si la salida esta activa.
 * @retval false Si la salida esta inactiva o la instancia no es valida.
 */
bool control_on_off_esta_salida_activa(const control_on_off_t* control);

/**
 * @brief Obtiene la ultima medicion procesada.
 *
 * @param control Instancia concreta.
 * @param medicion Variable de salida.
 *
 * @retval true Si habia una medicion valida para devolver.
 * @retval false Si la instancia es invalida o todavia no se proceso ninguna medicion.
 */
bool control_on_off_obtener_ultima_medicion(const control_on_off_t* control, int16_t* medicion);

#endif // CONTROL_CONTROL_ON_OFF_H_
