/*
 * led_Driver.h
 *
 *  Created on: 12 abr. 2023
 *      Author: agustin
 */

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#define LED0R 1
#define LED0G 2
#define LED0B 3
#define LED1 4
#define LED2 5
#define LED3 6

#include "chip.h"

void led_init(void);
void led_turn_on(uint8_t);
void led_turn_off(uint8_t);
void led_toggle(uint8_t);

#endif /* LED_DRIVER_H_ */
