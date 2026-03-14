/**
 * @file delay_driver.c
 * @brief Common busy-wait delays backed by LPCOpen stopwatch primitives
 */

#include "delay_driver.h"

#include <stopwatch.h>

static bool driver_delay_initialized_;

static void driver_delay_ensure_initialized(void)
{
    if (!driver_delay_initialized_) {
        StopWatch_Init();
        driver_delay_initialized_ = true;
    }
}

void driver_delay_init(void)
{
    driver_delay_ensure_initialized();
}

void driver_delay_us(uint32_t microseconds)
{
    driver_delay_ensure_initialized();
    StopWatch_DelayUs(microseconds);
}

void driver_delay_ms(uint32_t milliseconds)
{
    driver_delay_ensure_initialized();
    StopWatch_DelayMs(milliseconds);
}
