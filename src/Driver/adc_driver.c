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

ADC_CHANNEL_T adc_channel = ADC_CH0;
uint16_t adc_value = 0;

#define MAX_ADC_CHANNELS 8


void board_adc_init(uint8_t channel)
{
    ADC_CLOCK_SETUP_T adc_config;
    adc_config.adcRate = ADC_MAX_SAMPLE_RATE;
    adc_config.bitsAccuracy = ADC_10BITS;
    adc_config.burstMode = FALSE;

    board_adc_set_channel(channel);

    Chip_SCU_ADC_Channel_Config(0, adc_channel);
    Chip_ADC_Init(LPC_ADC0, &adc_config);
    Chip_ADC_EnableChannel(LPC_ADC0, adc_channel, ENABLE);
}


void board_adc_int_enable()
{
    NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_EnableIRQ(ADC0_IRQn);
    Chip_ADC_Int_SetChannelCmd(LPC_ADC0, adc_channel, ENABLE);
    Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}


void board_adc_set_channel(uint8_t channel)
{
    if (channel < MAX_ADC_CHANNELS) { adc_channel = (ADC_CHANNEL_T) channel; }
}

uint16_t board_adc_polling()
{
    Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
    while (Chip_ADC_ReadStatus(LPC_ADC0, adc_channel, ADC_DR_DONE_STAT) == RESET) {}
    Chip_ADC_ReadValue(LPC_ADC0, adc_channel, &adc_value);
    return adc_value;
}
