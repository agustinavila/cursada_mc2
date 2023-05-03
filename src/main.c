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

#if defined(__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "Driver/buzzer.h"
#include "Driver/led_Driver.h"
#include "Driver/teclado.h"
#include "Driver/teclasPinInt.h"
#include "Driver/teclas_driver.h"

#include <cr_section_macros.h>
#include <stdint.h>


int main(void)
{
#if defined(__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
#if defined(__MULTICORE_MASTER) || defined(__MULTICORE_NONE)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
#endif
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    //variable definitions
    static volatile long i = 0;
    uint8_t last_char = 0xFF;

    // Initialization
    led_init();
    buzzer_init();
    buzzer_apaga();
    buttons_init();
    board_keyboard_init();

    // Interrupts enabling
    board_keyboard_int_enable();
    button_int_enable(TECLA1);
    button_int_enable(TECLA2);
    button_int_enable(TECLA3);
    button_int_enable(TECLA4);

    // Infinite loop
    while (1) {
        i++;
        if (i > 1000000) {
            last_char = board_keyboard_get_last_char();
            switch (last_char) {
            case 0:
            case 10:
            case 20:
                led_toggle(LED1);
                break;
            case 1:
            case 11:
            case 21:
                led_toggle(LED2);
                break;
            case 2:
            case 12:
            case 22:
                led_toggle(LED3);
                break;
            case 30:
                led_toggle(LED0B);
                break;
            case 31:
                led_toggle(LED0G);
                break;
            case 32:
                led_toggle(LED0R);
                break;
            case 0xFF:
                break;
            }
            i = 0;
        }
    }
    return 0;
}

void PININT0_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
    led_toggle(LED0R);
}


void PININT1_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
    led_toggle(LED1);
}


void PININT2_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);
    led_toggle(LED2);
}


void PININT3_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
    led_toggle(LED3);
}
