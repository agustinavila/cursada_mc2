/**
 * @file buzzer.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(DRIVER_BUZZER_H)
#define DRIVER_BUZZER_H

#include "chip.h"

void buzzer_init();
void buzzer_turn_on();
void buzzer_turn_off();

#endif // DRIVER_BUZZER_H
