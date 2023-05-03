/**
 * @file adc_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "adc_driver.h"

static ADC_CLOCK_SETUP_T adc_config;
uint8_t adc_channel = 0;
uint16_t adc_value = 0;


void board_adc_init()
{
    adc_config.adcRate = ADC_MAX_SAMPLE_RATE;
    adc_config.bitsAccuracy = ADC_10BITS;
    adc_config.burstMode = FALSE;
}


void board_adc_int_enable()
{
    Chip_SCU_ADC_Channel_Config(0, ADC_CH1);
    Chip_ADC_Init(LPC_ADC0, &adc_config);
    Chip_ADC_EnableChannel(LPC_ADC0, ADC_CH0, ENABLE);
}


void board_adc_polling()
{
    Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
    while (Chip_ADC_ReadStatus(LPC_ADC0, adc_channel, ADC_DR_DONE_STAT) == RESET) {}
    Chip_ADC_ReadValue(LPC_ADC0, adc_channel, &adc_value);
}
