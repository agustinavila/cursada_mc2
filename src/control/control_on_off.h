/**
 * @file control_on_off.h
 * @brief Implementacion de control on/off con histeresis.
 */

#if !defined(CONTROL_CONTROL_ON_OFF_H_)
#define CONTROL_CONTROL_ON_OFF_H_

#include "control/control.h"

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
    int16_t consigna_deci_celsius;      // setpoint en decimas de grado Celsius
    uint16_t histeresis_deci_celsius;   // histeresis en decimas de grado Celsius
    bool habilitado;
} control_on_off_configuracion_t;

/**
 * @brief Instancia concreta del control on/off.
 *
 * Contiene una base comun para poder ser tratada de forma polimorfica a
 * traves de `control_generico_t`.
 */
typedef struct {
    control_generico_t base;
    control_on_off_configuracion_t configuracion;
    bool salida_activa;
    bool inicializado;
    bool tiene_medicion;
    int16_t ultima_medicion_deci_celsius;
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
 * @brief Devuelve la vista generica de una instancia concreta.
 *
 * @param control Instancia concreta.
 *
 * @return Puntero a la base generica para operar con la interfaz comun.
 */
control_generico_t* control_on_off_como_generico(control_on_off_t* control);

/**
 * @brief Devuelve la vista generica constante de una instancia concreta.
 *
 * @param control Instancia concreta.
 *
 * @return Puntero constante a la base generica.
 */
const control_generico_t* control_on_off_como_generico_const(const control_on_off_t* control);

#endif // CONTROL_CONTROL_ON_OFF_H_
