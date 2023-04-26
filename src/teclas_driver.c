/*
 * teclas_driver.c
 *
 *  Created on: 12 abr. 2023
 *      Author: agustin
 */

#include "teclas_driver.h"

void teclas_inicializar(void) {
	Chip_GPIO_Init(LPC_GPIO_PORT);
	Chip_SCU_PinMux(1, 0, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(1, 1, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(1, 2, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(1, 6, MD_PUP | MD_EZI | MD_ZI, FUNC0);

	Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 4), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 8), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 9), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 9), 0);
}

uint8_t teclas_leer_pin(uint8_t numero_tecla) {
	uint8_t tecla = 0;
	switch (numero_tecla) {
	case TECLA1:
		tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 4));
		break;
	case TECLA2:
		tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 8));
		break;
	case TECLA3:
		tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 9));
		break;
	case TECLA4:
		tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 1, 9));
		break;
	default:
		tecla = 0;
		break;
	}
	return tecla;
}

uint8_t teclas_leer_pines(void) {
	uint8_t teclas = 0;
	teclas = LeerTecla(TECLA1);
	teclas |= (LeerTecla(TECLA2) << 1);
    teclas |= (LeerTecla(TECLA3) << 2);
    teclas |= (LeerTecla(TECLA4) << 3);
    return teclas;
    }
