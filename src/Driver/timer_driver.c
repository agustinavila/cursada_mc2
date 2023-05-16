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

// uint32_t timer_counter_;

void board_timer_init(uint32_t timer_value_ms)
{
    Chip_RIT_Init(LPC_RITIMER);
    board_timer_set_period(timer_value_ms);

    NVIC_ClearPendingIRQ(RITIMER_IRQn);
    NVIC_EnableIRQ(RITIMER_IRQn);
    board_timer_clear_timer();
}

void board_timer_set_period(uint32_t timer_value_ms)
{ //
    Chip_RIT_SetTimerInterval(LPC_RITIMER, timer_value_ms);
}

// void board_timer_clear_timer(void)
// { //
//     timer_counter_ = 0;
// }

// uint32_t board_timer_get_value(void)
// { //
//     return timer_counter_;
// }
