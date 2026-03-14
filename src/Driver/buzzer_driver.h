/**
 * @file buzzer_driver.h
 * @brief Interfaz del driver de buzzer
 */

#if !defined(DRIVER_BUZZER_H)
#define DRIVER_BUZZER_H

#include "chip.h"

/**
 * @brief Inicializa el pin de control del buzzer.
 */
void buzzer_init(void);

/**
 * @brief Activa el buzzer.
 */
void buzzer_turn_on(void);

/**
 * @brief Desactiva el buzzer.
 */
void buzzer_turn_off(void);

#endif // DRIVER_BUZZER_H
