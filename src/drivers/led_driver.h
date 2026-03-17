/**
 * @file led_driver.h
 * @brief Interfaz del driver de LEDs discretos
 */

#if !defined(LED_DRIVER_H_)
#define LED_DRIVER_H_

#include "chip.h"

/** @brief LED rojo del LED RGB de placa. */
#define LED0R 1
/** @brief LED verde del LED RGB de placa. */
#define LED0G 2
/** @brief LED azul del LED RGB de placa. */
#define LED0B 3
/** @brief LED discreto 1. */
#define LED1 4
/** @brief LED discreto 2. */
#define LED2 5
/** @brief LED discreto 3. */
#define LED3 6

/**
 * @brief Inicializa los pines asociados a los LEDs.
 */
void led_init(void);

/**
 * @brief Enciende un LED.
 *
 * @param led_id Identificador del LED a encender.
 */
void led_turn_on(uint8_t led_id);

/**
 * @brief Apaga un LED.
 *
 * @param led_id Identificador del LED a apagar.
 */
void led_turn_off(uint8_t led_id);

/**
 * @brief Invierte el estado actual de un LED.
 *
 * @param led_id Identificador del LED a conmutar.
 */
void led_toggle(uint8_t led_id);

#endif // LED_DRIVER_H_
