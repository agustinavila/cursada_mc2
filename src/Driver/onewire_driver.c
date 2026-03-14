/**
 * @file onewire_driver.c
 * @brief Implementacion del driver para bus 1-Wire por bit-banging
 */

#include "onewire_driver.h"

#include "delay_driver.h"

static void onewire_drive_low(onewire_driver_t* driver)
{
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, driver->pin.gpio_port, driver->pin.gpio_pin);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, driver->pin.gpio_port, driver->pin.gpio_pin, true);
}

static void onewire_release_line(onewire_driver_t* driver)
{
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, driver->pin.gpio_port, driver->pin.gpio_pin, false);
}

static bool onewire_read_line(onewire_driver_t* driver)
{
    return (bool) Chip_GPIO_GetPinState(LPC_GPIO_PORT, driver->pin.gpio_port, driver->pin.gpio_pin);
}

bool onewire_init(onewire_driver_t* driver, const onewire_pin_config_t* pin_config)
{
    if ((driver == 0) || (pin_config == 0)) {
        return false;
    }

    driver->pin = *pin_config;

    Chip_SCU_PinMux(driver->pin.scu_port,
                    driver->pin.scu_pin,
                    driver->pin.scu_mode,
                    driver->pin.scu_func);

    driver_delay_init();
    onewire_release_line(driver);

    driver->initialized = true;
    return true;
}

bool onewire_reset(onewire_driver_t* driver)
{
    bool presence_detected = false;

    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    onewire_drive_low(driver);
    driver_delay_us(480U);
    onewire_release_line(driver);
    driver_delay_us(70U);

    /* Presence pulse is active low after the master releases the bus. */
    presence_detected = !onewire_read_line(driver);
    driver_delay_us(410U);

    return presence_detected;
}

void onewire_write_bit(onewire_driver_t* driver, bool bit_value)
{
    if ((driver == 0) || !driver->initialized) {
        return;
    }

    onewire_drive_low(driver);

    if (bit_value) {
        driver_delay_us(6U);
        onewire_release_line(driver);
        driver_delay_us(64U);
    } else {
        driver_delay_us(60U);
        onewire_release_line(driver);
        driver_delay_us(10U);
    }
}

bool onewire_read_bit(onewire_driver_t* driver)
{
    bool bit_value = false;

    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    onewire_drive_low(driver);
    driver_delay_us(6U);
    onewire_release_line(driver);
    driver_delay_us(9U);

    bit_value = onewire_read_line(driver);
    driver_delay_us(55U);

    return bit_value;
}

void onewire_write_byte(onewire_driver_t* driver, uint8_t value)
{
    uint8_t bit_index = 0U;

    if ((driver == 0) || !driver->initialized) {
        return;
    }

    for (bit_index = 0U; bit_index < 8U; ++bit_index) {
        onewire_write_bit(driver, (bool) (value & 0x01U));
        value >>= 1U;
    }
}

uint8_t onewire_read_byte(onewire_driver_t* driver)
{
    uint8_t bit_index = 0U;
    uint8_t value = 0U;

    if ((driver == 0) || !driver->initialized) {
        return 0U;
    }

    for (bit_index = 0U; bit_index < 8U; ++bit_index) {
        if (onewire_read_bit(driver)) {
            value |= (uint8_t) (1U << bit_index);
        }
    }

    return value;
}
