/*
 * teclas_driver.h
 *
 *  Created on: 12 abr. 2023
 *      Author: agustin
 */

#ifndef TECLAS_DRIVER_H_
#define TECLAS_DRIVER_H_

#define TECLA1 1
#define TECLA2 2
#define TECLA3 3
#define TECLA4 4

#include "chip.h"

void buttons_init(void);
uint8_t teclas_leer_pin(uint8_t);
uint8_t teclas_leer_pines(void);

#endif /* TECLAS_DRIVER_H_ */
