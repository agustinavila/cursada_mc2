/**
 * @file buttons_driver.h
 * @brief Interfaz del driver de pulsadores discretos
 */

#if !defined(TECLAS_DRIVER_H_)
#define TECLAS_DRIVER_H_

#include "chip.h"

/** @brief Identificador de la tecla 1. */
#define TECLA1 1
/** @brief Identificador de la tecla 2. */
#define TECLA2 2
/** @brief Identificador de la tecla 3. */
#define TECLA3 3
/** @brief Identificador de la tecla 4. */
#define TECLA4 4

/** @brief Alias del handler de interrupcion del pulsador 1. */
#define PININT0_IRQ_HANDLER GPIO0_IRQHandler
/** @brief Alias del handler de interrupcion del pulsador 2. */
#define PININT1_IRQ_HANDLER GPIO1_IRQHandler
/** @brief Alias del handler de interrupcion del pulsador 3. */
#define PININT2_IRQ_HANDLER GPIO2_IRQHandler
/** @brief Alias del handler de interrupcion del pulsador 4. */
#define PININT3_IRQ_HANDLER GPIO3_IRQHandler

/**
 * @brief Inicializa los pines asociados a los cuatro pulsadores.
 */
void buttons_init(void);

/**
 * @brief Notifica al driver que una tecla genero una interrupcion externa.
 *
 * Esta funcion debe llamarse desde el handler asociado a la tecla
 * correspondiente.
 *
 * @param button_id Identificador de la tecla que disparo la IRQ.
 */
void button_notify_irq(uint8_t button_id);

/**
 * @brief Procesa el debounce y actualiza los eventos latcheados de teclas.
 *
 * Debe llamarse de manera periodica desde el lazo principal.
 *
 * @param delta_ms Tiempo transcurrido desde la ultima llamada, en milisegundos.
 */
void buttons_process(uint32_t delta_ms);

/**
 * @brief Devuelve y consume el siguiente evento de tecla confirmado.
 *
 * @return `TECLA1`..`TECLA4` si hay un evento pendiente, o `0` si no hay
 * eventos para consumir.
 */
uint8_t button_get_event(void);

#endif // TECLAS_DRIVER_H_
