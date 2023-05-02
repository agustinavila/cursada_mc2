/**
 * @file main.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

#include "Driver/led_Driver.h"
#include "Driver/teclas_driver.h"
#include "Driver/teclasPinInt.h"
#include "Driver/teclado.h"
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

	led_init();
	buttons_init();
	board_keyboard_init();
    board_Keyboard_int_enable();
	button_int_enable(TECLA1);
	button_int_enable(TECLA2);
	button_int_enable(TECLA3);
	button_int_enable(TECLA4);
	// Force the counter to be placed into memory
	static volatile long i = 0;
	uint8_t keyboard_matrix[KEYBOARD_MAX_ROWS][KEYBOARD_MAX_COLUMNS];
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
		if (i > 10000) {
			board_keyboard_read_matrix(*keyboard_matrix);
			if(keyboard_matrix[0][0]>=1){
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

