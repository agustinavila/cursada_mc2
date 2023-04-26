/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

#include "led_Driver.h"
#include "teclas_driver.h"
#include "teclasPinInt.h"
#include "teclado.h"
#include <stdint.h>

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
#if defined (__MULTICORE_MASTER) || defined (__MULTICORE_NONE)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
#endif
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

	led_inicializar();
	teclas_inicializar();
	Board_Keyboard_Init();
    Board_Keyboard_IntEnable();
	teclas_Init_Int(TECLA1);
	teclas_Init_Int(TECLA2);
	teclas_Init_Int(TECLA3);
	teclas_Init_Int(TECLA4);
	// Force the counter to be placed into memory
	volatile static long i = 0;
	uint8_t teclado_matriz[KEYBOARD_MAX_ROWS][KEYBOARD_MAX_COLUMNS];
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
		if (i > 10000) {
			Board_Keyboard_readMatrix(&teclado_matriz);
			if(teclado_matriz[0][0]>=1){
				led_toggle(LED1);
			}
			i = 0;
		}
	}
	return 0;
}

void PININT0_IRQ_HANDLER(void) {
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	led_toggle(LED0R);
}
void PININT1_IRQ_HANDLER(void) {
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
	led_toggle(LED1);
}
void PININT2_IRQ_HANDLER(void) {
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);
	led_toggle(LED2);
}
void PININT3_IRQ_HANDLER(void) {
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
	led_toggle(LED3);
}

