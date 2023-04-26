/*
 * led_Driver.c
 *
 *  Created on: 12 abr. 2023
 *      Author: agustin
 */

#include "led_Driver.h"

void led_inicializar(void) {
	Chip_GPIO_Init(LPC_GPIO_PORT);
	Chip_SCU_PinMux(2, 0, MD_PUP, FUNC4);
	Chip_SCU_PinMux(2, 1, MD_PUP, FUNC4);
	Chip_SCU_PinMux(2, 2, MD_PUP, FUNC4);
	Chip_SCU_PinMux(2, 10, MD_PUP, FUNC0);
	Chip_SCU_PinMux(2, 11, MD_PUP, FUNC0);
	Chip_SCU_PinMux(2, 12, MD_PUP, FUNC0);

	Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1 << 0), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1 << 1), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1 << 2), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 14), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 11), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 12), 1);
}

void led_prender(uint8_t numero_led) {
	switch (numero_led) {

	case LED0R:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 0);
		break;
	case LED0G:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 1);
		break;
	case LED0B:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 2);
		break;
	case LED1:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 0, 14);
		break;
	case LED2:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 11);
		break;
	case LED3:
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 12);
		break;
	default:
		break;
	}
}

void led_apagar(uint8_t numero_led) {
	switch (numero_led) {

	case LED0R:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 0);
		break;
	case LED0G:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 1);
		break;
	case LED0B:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 2);
		break;
	case LED1:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 0, 14);
		break;
	case LED2:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 1, 11);
		break;
	case LED3:
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 1, 12);
		break;
	default:
		break;
	}
}

void led_toggle(uint8_t numero_led){
	switch (numero_led) {

		case LED0R:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 5, 0);
			break;
		case LED0G:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 5, 1);
			break;
		case LED0B:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 5, 2);
			break;
		case LED1:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 0, 14);
			break;
		case LED2:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 1, 11);
			break;
		case LED3:
			Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 1, 12);
			break;
		default:
			break;
		}
}
