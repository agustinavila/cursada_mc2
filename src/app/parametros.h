/**
 * @file parametros.h
 * @brief Persistencia y administracion de parametros de la aplicacion.
 */

#if !defined(APP_PARAMETROS_H_)
#define APP_PARAMETROS_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t setpoint_deci_celsius;         // setpoint persistido en decimas de grado Celsius
    uint16_t histeresis_deci_celsius;      // histeresis persistida en decimas de grado Celsius
    uint32_t tiempo_minimo_encendido_ms;   // permanencia minima de la salida en estado encendido
    uint32_t tiempo_minimo_apagado_ms;     // permanencia minima de la salida en estado apagado
    bool modo_calentar;                    // sentido actual del control on/off
} parametros_control_t;

typedef struct {
    parametros_control_t control;
} parametros_t;

bool parametros_init(void);
const parametros_t* parametros_obtener(void);
bool parametros_guardar(void);
void parametros_restablecer_defaults(void);
bool parametros_actualizar_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   uint32_t tiempo_minimo_encendido_ms,
                                   uint32_t tiempo_minimo_apagado_ms,
                                   bool modo_calentar);

#endif // APP_PARAMETROS_H_
