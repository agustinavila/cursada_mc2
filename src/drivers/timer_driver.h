/**
 * @file timer_driver.h
 * @brief Interfaz del driver del temporizador RIT
 */

#if !defined(DRIVER_TIMER_DRIVER_H)
#define DRIVER_TIMER_DRIVER_H

#include <chip.h>

/** @brief Alias del handler del temporizador RIT. */
#define RIT_Handler RIT_IRQHandler

/**
 * @brief Inicializa y arranca el temporizador RIT con un periodo dado.
 *
 * @param timer_value_ms Periodo del temporizador en milisegundos.
 */
void board_timer_init(uint32_t timer_value_ms);

/**
 * @brief Cambia el periodo del temporizador RIT.
 *
 * @param timer_value Nuevo periodo en milisegundos.
 */
void board_timer_set_period(uint32_t timer_value);

#endif // DRIVER_TIMER_DRIVER_H
