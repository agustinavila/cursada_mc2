/**
 * @file parametros.h
 * @brief Persistencia y administracion de parametros de la aplicacion.
 */

#if !defined(APP_PARAMETROS_H_)
#define APP_PARAMETROS_H_

#include <stdbool.h>
#include <stdint.h>

/** @brief Tamano del codigo ROM 1-Wire persistido para identificar sensores. */
#define PARAMETROS_SENSOR_ROM_SIZE 8U

typedef struct {
    int16_t setpoint_deci_celsius;
    uint16_t histeresis_deci_celsius;
    bool modo_calentar;
} parametros_control_t;

typedef enum {
    PARAMETROS_SENSOR_MODO_AUTO = 0, /**< Usa el unico sensor si solo hay uno presente. */
    PARAMETROS_SENSOR_MODO_ROM,      /**< Busca un sensor especifico por su codigo ROM. */
} parametros_sensor_modo_t;

typedef struct {
    parametros_sensor_modo_t modo;
    bool rom_valida;
    uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE];
} parametros_sensor_t;

typedef struct {
    parametros_control_t control;
    parametros_sensor_t sensor_proceso;
} parametros_t;

bool parametros_init(void);
const parametros_t* parametros_obtener(void);
bool parametros_guardar(void);
void parametros_restablecer_defaults(void);
bool parametros_actualizar_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   bool modo_calentar);
bool parametros_configurar_sensor_proceso_auto(void);
bool parametros_configurar_sensor_proceso_por_rom(const uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE]);

#endif // APP_PARAMETROS_H_
