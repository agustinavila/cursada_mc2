/**
 * @file dac_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "dac_driver.h"

#include "timer_driver.h"


uint16_t amplitude_max = 0;
uint16_t period_max = 0;
uint16_t rate_max = 0;

void board_dac_init(uint8_t channel)
{
    Chip_SCU_DAC_Analog_Config();
    Chip_DAC_Init(LPC_DAC);
    Chip_DAC_ConfigDAConverterControl(LPC_DAC, DAC_DMA_ENA);
    Chip_DAC_UpdateValue(LPC_DAC, 0); //0->0V -- 1024->3.3V
}

void board_dac_set_value_mv(uint16_t value_mv)
{
    //0->0V -- 1024->3300mV
    const uint16_t dac_value = value_mv / 3;
    Chip_DAC_UpdateValue(LPC_DAC, dac_value);
}

void board_dac_triangle_set(uint16_t amplitude, uint16_t period)
{
    amplitude_max = amplitude;
    period_max = period;
    rate_max = amplitude_max / period_max;
    board_timer_clear_timer();
}

uint16_t board_dac_triangle_update()
{
    static uint32_t prev_timer_val;
    static uint16_t value_tmp;
    const uint32_t timer_val = board_timer_get_value();
    if (prev_timer_val != timer_val) { //Esta condicion permite usarlo por polling o interrupcion
        value_tmp = rate_max * timer_val;
        ActualizarDac_mV(value_tmp);
    }
    if (timer_val == period_max) { board_timer_clear_timer(); }
}