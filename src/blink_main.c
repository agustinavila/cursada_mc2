#include "chip.h"

#include <stdint.h>

static void blink_led_init(void)
{
    Chip_GPIO_Init(LPC_GPIO_PORT);
    Chip_SCU_PinMux(2, 12, MD_PUP, FUNC0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1U << 12), true);
}

static void delay_cycles(volatile uint32_t cycles)
{
    while (cycles-- > 0U) {
        __asm volatile ("nop");
    }
}

int main(void)
{
    SystemCoreClockUpdate();
    blink_led_init();

    while (1) {
        Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, 1, 12);
        delay_cycles(SystemCoreClock / 12U);
    }
}
