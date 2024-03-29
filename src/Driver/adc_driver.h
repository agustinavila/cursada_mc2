/**
 * @file adc_driver.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "chip.h"

#if !defined(ADC_DRIVER_H)
#define ADC_DRIVER_H

void board_adc_init(uint8_t channel);

void board_adc_int_enable();

void board_adc_set_channel(uint8_t channel);

uint16_t board_adc_polling();


#endif // ADC_DRIVER_H
