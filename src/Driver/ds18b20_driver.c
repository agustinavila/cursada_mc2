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

static bool ds18b20_finish_conversion(ds18b20_driver_t* driver)
{
    uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];

    if (!ds18b20_read_scratchpad(driver, scratchpad)) {
        driver->state = DS18B20_STATE_ERROR;
        driver->sample_valid = false;
        return false;
    }

    driver->last_raw_temperature = (int16_t) (((uint16_t) scratchpad[1] << 8U) | scratchpad[0]);
    driver->sample_valid = true;
    driver->state = DS18B20_STATE_DATA_READY;
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

    driver->sample_valid = false;
    driver->last_raw_temperature = 0;
    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_STATE_IDLE;
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
    if (!ds18b20_begin_command(driver, DS18B20_CMD_CONVERT_T)) {
        driver->state = DS18B20_STATE_ERROR;
        return false;
    }

    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_STATE_CONVERTING;
    return true;
}

void ds18b20_process(ds18b20_driver_t* driver, uint16_t elapsed_ms)
{
    if ((driver == 0) || !driver->initialized) {
        return;
    }

    if (driver->state != DS18B20_STATE_CONVERTING) {
        return;
    }

    if ((uint32_t) driver->conversion_elapsed_ms + elapsed_ms >= DS18B20_CONVERSION_TIME_MS) {
        driver->conversion_elapsed_ms = DS18B20_CONVERSION_TIME_MS;
        (void) ds18b20_finish_conversion(driver);
    } else {
        driver->conversion_elapsed_ms = (uint16_t) (driver->conversion_elapsed_ms + elapsed_ms);
    }
}

bool ds18b20_is_busy(const ds18b20_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    return (driver->state == DS18B20_STATE_CONVERTING);
}

bool ds18b20_has_valid_sample(const ds18b20_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    return driver->sample_valid;
}

bool ds18b20_get_latest_raw(const ds18b20_driver_t* driver, int16_t* raw_temperature)
{
    if ((driver == 0) || (raw_temperature == 0) || !driver->initialized || !driver->sample_valid) {
        return false;
    }

    *raw_temperature = driver->last_raw_temperature;
    return true;
}

bool ds18b20_get_latest_temperature_celsius(const ds18b20_driver_t* driver,
                                            float* temperature_celsius)
{
    if ((temperature_celsius == 0) || !ds18b20_has_valid_sample(driver)) {
        return false;
    }

    *temperature_celsius = (float) driver->last_raw_temperature / 16.0f;
    return true;
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
    if ((driver == 0) || (raw_temperature == 0)) {
        return false;
    }

    if (!ds18b20_start_conversion(driver)) {
        return false;
    }

    driver_delay_ms(DS18B20_CONVERSION_TIME_MS);

    if (!ds18b20_finish_conversion(driver)) {
        return false;
    }

    return ds18b20_get_latest_raw(driver, raw_temperature);
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
