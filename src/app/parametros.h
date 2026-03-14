/**
 * @file parametros.h
 * @brief Persistencia y administracion de parametros de la aplicacion.
 */

#if !defined(APP_PARAMETROS_H_)
#define APP_PARAMETROS_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t setpoint_deci_celsius;
    uint16_t histeresis_deci_celsius;
    bool modo_calentar;
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
                                   bool modo_calentar);

#endif // APP_PARAMETROS_H_
