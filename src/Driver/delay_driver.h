/**
 * @file delay_driver.h
 * @brief Common busy-wait delays backed by LPCOpen stopwatch primitives
 */

#if !defined(DRIVER_DELAY_DRIVER_H_)
#define DRIVER_DELAY_DRIVER_H_

#include <chip.h>

void driver_delay_init(void);

void driver_delay_us(uint32_t microseconds);

void driver_delay_ms(uint32_t milliseconds);

#endif // DRIVER_DELAY_DRIVER_H_
