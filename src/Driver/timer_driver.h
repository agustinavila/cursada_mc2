/**
 * @file timer_driver.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(DRIVER_TIMER_DRIVER_H)
#define DRIVER_TIMER_DRIVER_H

#include <chip.h>

#define RIT_Handler RIT_IRQHandler

/**
 * @brief sets and starts the RI Timer with the provided value
 * 
 * @param timer_value_ms timer value in miliseconds
 */
void board_timer_init(uint32_t timer_value_ms);

void board_timer_set_period(uint32_t timer_value);

// void board_timer_clear_timer(void);

// uint32_t board_timer_get_value(void);


#endif // DRIVER_TIMER_DRIVER_H
