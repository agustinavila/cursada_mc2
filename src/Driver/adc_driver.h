/**
 * @file adc_driver.h
 * @brief Interfaz del driver ADC
 */

#if !defined(ADC_DRIVER_H)
#define ADC_DRIVER_H

#include "chip.h"

/**
 * @brief Inicializa el ADC y selecciona el canal de entrada inicial.
 *
 * @param channel Canal ADC a utilizar.
 */
void board_adc_init(uint8_t channel);

/**
 * @brief Habilita las interrupciones del ADC.
 */
void board_adc_int_enable(void);

/**
 * @brief Cambia el canal ADC activo.
 *
 * @param channel Nuevo canal ADC a seleccionar.
 */
void board_adc_set_channel(uint8_t channel);

/**
 * @brief Realiza una conversion ADC por polling y devuelve el resultado.
 *
 * @return Muestra convertida por el ADC.
 */
uint16_t board_adc_polling(void);

#endif // ADC_DRIVER_H
