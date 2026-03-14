/**
 * @file delay_driver.h
 * @brief Interfaz comun de delays bloqueantes basada en stopwatch de LPCOpen
 */

#if !defined(DRIVER_DELAY_DRIVER_H_)
#define DRIVER_DELAY_DRIVER_H_

#include <chip.h>

/**
 * @brief Inicializa la base de tiempos usada por los delays bloqueantes.
 *
 * Esta funcion puede llamarse mas de una vez. La inicializacion real se hace
 * una sola vez de manera interna.
 */
void driver_delay_init(void);

/**
 * @brief Realiza una espera bloqueante expresada en microsegundos.
 *
 * @param microseconds Tiempo de espera en microsegundos.
 */
void driver_delay_us(uint32_t microseconds);

/**
 * @brief Realiza una espera bloqueante expresada en milisegundos.
 *
 * @param milliseconds Tiempo de espera en milisegundos.
 */
void driver_delay_ms(uint32_t milliseconds);

#endif // DRIVER_DELAY_DRIVER_H_
