/**
 * @file timer_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "timer_driver.h"

static volatile uint32_t board_timer_ticks_ = 0U;

void board_timer_init(uint32_t timer_value_ms)
{
    Chip_RIT_Init(LPC_RITIMER);
    board_timer_set_period(timer_value_ms);
    board_timer_ticks_ = 0U;

    NVIC_ClearPendingIRQ(RITIMER_IRQn);
    NVIC_EnableIRQ(RITIMER_IRQn);
}

void board_timer_set_period(uint32_t timer_value_ms)
{ //
    Chip_RIT_SetTimerInterval(LPC_RITIMER, timer_value_ms);
}

void board_timer_irq_handler(void)
{
    board_timer_ticks_++;
    Chip_RIT_ClearInt(LPC_RITIMER);
}

uint32_t board_timer_get_ticks(void)
{
    return board_timer_ticks_;
}
