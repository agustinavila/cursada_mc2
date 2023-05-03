/*
 * teclas_driver.h
 *
 *  Created on: 12 abr. 2023
 *      Author: agustin
 */

#ifndef TECLAS_DRIVER_H_
#define TECLAS_DRIVER_H_

#include "chip.h"

#define TECLA1 1
#define TECLA2 2
#define TECLA3 3
#define TECLA4 4

#define PININT0_IRQ_HANDLER GPIO0_IRQHandler
#define PININT1_IRQ_HANDLER GPIO1_IRQHandler
#define PININT2_IRQ_HANDLER GPIO2_IRQHandler
#define PININT3_IRQ_HANDLER GPIO3_IRQHandler


void buttons_init();

uint8_t button_read_pin(uint8_t);

uint8_t button_read_all_pins();

void button_int_enable(uint8_t);

#endif /* TECLAS_DRIVER_H_ */
