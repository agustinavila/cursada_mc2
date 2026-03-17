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

/**
 * @brief Handler de bajo nivel del RIT para mantener el tick del sistema.
 *
 * Debe llamarse desde la ISR asociada al temporizador.
 */
void board_timer_irq_handler(void);

/**
 * @brief Devuelve la cantidad de ticks acumulados por el RIT.
 *
 * Cuando el temporizador se configura con periodo de 1 ms, este valor se
 * interpreta como tiempo en milisegundos desde la inicializacion.
 *
 * @return Contador monotono de ticks.
 */
uint32_t board_timer_get_ticks(void);

#endif // DRIVER_TIMER_DRIVER_H
