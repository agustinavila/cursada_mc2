/**
 * @file ds18b20_driver.c
 * @brief Implementacion del driver para sensor de temperatura DS18B20
 */

#include "ds18b20_driver.h"

#include "delay_driver.h"

#define DS18B20_CMD_SKIP_ROM 0xCCU
#define DS18B20_CMD_CONVERT_T 0x44U
#define DS18B20_CMD_READ_SCRATCHPAD 0xBEU

static uint8_t ds18b20_crc8(const uint8_t* data, uint8_t length)
{
    uint8_t index = 0U;
    uint8_t bit_index = 0U;
    uint8_t crc = 0U;

    for (index = 0U; index < length; ++index) {
        uint8_t current_byte = data[index];
        for (bit_index = 0U; bit_index < 8U; ++bit_index) {
            const uint8_t mix = (uint8_t) ((crc ^ current_byte) & 0x01U);
            crc >>= 1U;
            if (mix != 0U) {
                crc ^= 0x8CU;
            }
            current_byte >>= 1U;
        }
    }

    return crc;
}

static bool ds18b20_begin_command(ds18b20_driver_t* driver, uint8_t command)
{
    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    if (!onewire_reset(&driver->bus)) {
        return false;
    }

    onewire_write_byte(&driver->bus, DS18B20_CMD_SKIP_ROM);
    onewire_write_byte(&driver->bus, command);
    return true;
}

bool ds18b20_init(ds18b20_driver_t* driver, const onewire_pin_config_t* pin_config)
{
    if ((driver == 0) || (pin_config == 0)) {
        return false;
    }

    if (!onewire_init(&driver->bus, pin_config)) {
        return false;
    }

    driver->initialized = ds18b20_is_present(driver);
    return driver->initialized;
}

bool ds18b20_is_present(ds18b20_driver_t* driver)
{
    if ((driver == 0) || !driver->bus.initialized) {
        return false;
    }

    return onewire_reset(&driver->bus);
}

bool ds18b20_start_conversion(ds18b20_driver_t* driver)
{
    return ds18b20_begin_command(driver, DS18B20_CMD_CONVERT_T);
}

bool ds18b20_read_scratchpad(ds18b20_driver_t* driver, uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE])
{
    uint8_t index = 0U;

    if ((driver == 0) || (scratchpad == 0)) {
        return false;
    }

    if (!ds18b20_begin_command(driver, DS18B20_CMD_READ_SCRATCHPAD)) {
        return false;
    }

    for (index = 0U; index < DS18B20_SCRATCHPAD_SIZE; ++index) {
        scratchpad[index] = onewire_read_byte(&driver->bus);
    }

    return (ds18b20_crc8(scratchpad, DS18B20_SCRATCHPAD_SIZE) == 0U);
}

bool ds18b20_read_raw(ds18b20_driver_t* driver, int16_t* raw_temperature)
{
    uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];

    if ((driver == 0) || (raw_temperature == 0)) {
        return false;
    }

    if (!ds18b20_start_conversion(driver)) {
        return false;
    }

    /* At 12-bit resolution the maximum conversion time is 750 ms. */
    driver_delay_ms(750U);

    if (!ds18b20_read_scratchpad(driver, scratchpad)) {
        return false;
    }

    *raw_temperature = (int16_t) (((uint16_t) scratchpad[1] << 8U) | scratchpad[0]);
    return true;
}

bool ds18b20_read_temperature_celsius(ds18b20_driver_t* driver, float* temperature_celsius)
{
    int16_t raw_temperature = 0;

    if ((driver == 0) || (temperature_celsius == 0)) {
        return false;
    }

    if (!ds18b20_read_raw(driver, &raw_temperature)) {
        return false;
    }

    *temperature_celsius = (float) raw_temperature / 16.0f;
    return true;
}
