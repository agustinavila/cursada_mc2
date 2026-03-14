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

#include "app/app.h"

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
    app_init();
    while (1) {
        app_process();
    }

    return 0;
}

void PININT0_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
}


void PININT1_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
}


void PININT2_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);
}


void PININT3_IRQ_HANDLER(void)
{
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
}


void RIT_Handler(void)
{
    NVIC_ClearPendingIRQ(RITIMER_IRQn);
    NVIC_EnableIRQ(RITIMER_IRQn);
    Chip_RIT_ClearInt(LPC_RITIMER);
}
