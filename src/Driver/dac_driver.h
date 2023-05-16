/**
 * @file dac_driver.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#if !defined(DRIVER_DAC_DRIVER_H)
#define DRIVER_DAC_DRIVER_H

#include <chip.h>

void board_dac_init(uint8_t channel);

void board_dac_set_value_mv(uint16_t value_mv);

void board_dac_triangle_set(uint16_t amplitude, uint16_t period);

uint16_t board_dac_triangle_update();


#endif // DRIVER_DAC_DRIVER_H
