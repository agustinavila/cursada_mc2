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
 * @brief Lee el estado actual de un pulsador.
 *
 * @param button_id Identificador de la tecla a consultar.
 *
 * @retval 1 Si la tecla esta presionada.
 * @retval 0 Si la tecla no esta presionada.
 */
uint8_t button_read_pin(uint8_t button_id);

/**
 * @brief Lee simultaneamente el estado de los cuatro pulsadores.
 *
 * El resultado se devuelve como mascara de bits:
 * - bit 0: TECLA1
 * - bit 1: TECLA2
 * - bit 2: TECLA3
 * - bit 3: TECLA4
 *
 * @return Mascara con el estado actual de las teclas.
 */
uint8_t button_read_all_pins(void);

/**
 * @brief Habilita la interrupcion externa asociada a un pulsador.
 *
 * @param button_id Identificador de la tecla a configurar.
 */
void button_int_enable(uint8_t button_id);

#endif // TECLAS_DRIVER_H_
