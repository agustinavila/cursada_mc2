/**
 * @file teclado.c
 * @author Agustin Avila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "teclado.h"
//#include "puertos.h"
#include <string.h>

#define KEYBOARD_MAX_ROWS 4
#define KEYBOARD_MAX_COLUMNS 3

#define FILA0_IRQ_HANDLER GPIO4_IRQHandler
#define FILA1_IRQ_HANDLER GPIO5_IRQHandler
#define FILA2_IRQ_HANDLER GPIO6_IRQHandler
#define FILA3_IRQ_HANDLER GPIO7_IRQHandler

uint8_t stateMatrix[KEYBOARD_MAX_ROWS][KEYBOARD_MAX_COLUMNS];

void Board_Keyboard_Init(void) {
	Chip_GPIO_Init(LPC_GPIO_PORT);
	Chip_SCU_PinMux(4, 0, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(4, 1, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(4, 2, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(4, 3, MD_PUP | MD_EZI | MD_ZI, FUNC0);
	Chip_SCU_PinMux(1, 5, MD_PUP, FUNC0);
	Chip_SCU_PinMux(7, 4, MD_PUP, FUNC0);
	Chip_SCU_PinMux(7, 5, MD_PUP, FUNC0);

	Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 0), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 1), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 2), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 3), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 8), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 12), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 4, (1 << 13), 1);
}

void Board_Keyboard_IntEnable(void) {
	Chip_SCU_GPIOIntPinSel(4, 2, 0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH4);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH4);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH4);
	NVIC_ClearPendingIRQ(PIN_INT4_IRQn);
	NVIC_EnableIRQ(PIN_INT4_IRQn);

	Chip_SCU_GPIOIntPinSel(5, 2, 1);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH5);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH5);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH5);
	NVIC_ClearPendingIRQ(PIN_INT5_IRQn);
	NVIC_EnableIRQ(PIN_INT5_IRQn);

	Chip_SCU_GPIOIntPinSel(6, 2, 2);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH6);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH6);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH6);
	NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
	NVIC_EnableIRQ(PIN_INT6_IRQn);

	Chip_SCU_GPIOIntPinSel(7, 2, 3);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH7);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH7);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH7);
	NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
	NVIC_EnableIRQ(PIN_INT7_IRQn);
}

int Board_Keyboard_readCell(uint8_t fila, uint8_t columna) {
	return stateMatrix[fila][columna];
}

void Board_Keyboard_readMatrix(uint8_t *matrix_p) {
//	for (uint8_t row = 0; row < KEYBOARD_MAX_ROWS; row++) {
//		for (uint8_t col = 0; col < KEYBOARD_MAX_COLUMNS; col++) {
//			matrix_p[row][col] = stateMatrix[row][col];
//		}
//	}
	memcpy((char*)matrix_p, (char*)&stateMatrix, sizeof(uint8_t)*KEYBOARD_MAX_COLUMNS*KEYBOARD_MAX_ROWS);
}

void read_row(uint8_t row) {
	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 12);
	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 13);
	stateMatrix[row][0] = (uint8_t) Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 12);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 13);

	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 1, 8);
	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 13);
	stateMatrix[row][1] = (uint8_t) Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 8);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 13);

	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 1, 8);
	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 12);
	stateMatrix[row][2] = (uint8_t) Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 8);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 12);
}

void FILA0_IRQ_HANDLER(void) {
	Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 4);
	NVIC_ClearPendingIRQ(PIN_INT4_IRQn);
	read_row(0);
}

void FILA1_IRQ_HANDLER(void) {
	Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 5);
	NVIC_ClearPendingIRQ(PIN_INT5_IRQn);
	read_row(1);
}

void FILA2_IRQ_HANDLER(void) {
	Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 6);
	NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
	read_row(2);
}

void FILA3_IRQ_HANDLER(void) {
	Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 7);
	NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
	read_row(3);
}
