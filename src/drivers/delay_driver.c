/**
 * @file delay_driver.c
 * @brief Implementacion de delays bloqueantes basada en stopwatch de LPCOpen
 */

#include "delay_driver.h"

#include <stopwatch.h>

static bool driver_delay_initialized_;

static void driver_delay_ensure_initialized(void)
{
    /** @brief La base de tiempos se inicializa una sola vez de forma perezosa. */
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
