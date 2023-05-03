/**
 * @file buzzer.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "buzzer_driver.h"

void buzzer_init()
{
    Chip_GPIO_Init(LPC_GPIO_PORT);
    Chip_SCU_PinMux(6, 12, MD_PUP, FUNC0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 8), 1);
}


void buzzer_turn_on() { Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 8); }


void buzzer_turn_off() { Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 8); }
