/**
 * @file teclasPinInt.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "Driver/teclasPinInt.h"
#include "Driver/teclas_driver.h"

void button_int_enable(uint8_t tecla) {

	switch (tecla) {
	case TECLA1:
		Chip_SCU_GPIOIntPinSel(0, 0, 4);
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);

		NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
		NVIC_EnableIRQ(PIN_INT0_IRQn);
		break;

	case TECLA2:
		Chip_SCU_GPIOIntPinSel(1, 0, 8);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1);
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);

		NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
		NVIC_EnableIRQ(PIN_INT1_IRQn);
		break;

	case TECLA3:
		Chip_SCU_GPIOIntPinSel(2, 0, 9);
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);

		NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
		NVIC_EnableIRQ(PIN_INT2_IRQn);
		break;

	case TECLA4:
		Chip_SCU_GPIOIntPinSel(3, 1, 9);
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH3);
		Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);

		NVIC_ClearPendingIRQ(PIN_INT3_IRQn);
		NVIC_EnableIRQ(PIN_INT3_IRQn);
		break;

	default:
		break;
	}
}
