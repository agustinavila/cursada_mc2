/**
 * @file ds18b20_driver.c
 * @brief Implementacion del driver para sensor de temperatura DS18B20
 */

#include "ds18b20_driver.h"

#include "delay_driver.h"

#include <string.h>

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

static bool ds18b20_crc8_is_valid(const uint8_t* data, uint8_t length)
{
    return (ds18b20_crc8(data, length) == 0U);
}

static void ds18b20_reset_state(ds18b20_driver_t* driver)
{
    driver->sample_valid = false;
    driver->last_raw_temperature = 0;
    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_STATE_IDLE;
}

static bool ds18b20_begin_command(ds18b20_driver_t* driver, uint8_t command)
{
    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    if (!onewire_reset(&driver->bus)) {
        return false;
    }

    if (driver->use_match_rom) {
        onewire_match_rom(&driver->bus, driver->rom_code);
    } else {
        onewire_skip_rom(&driver->bus);
    }
    onewire_write_byte(&driver->bus, command);
    return true;
}

static bool ds18b20_bus_begin_command(onewire_driver_t* bus,
                                      const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE],
                                      uint8_t command)
{
    if ((bus == 0) || !bus->initialized) {
        return false;
    }

    if (!onewire_reset(bus)) {
        return false;
    }

    if (rom_code != 0) {
        onewire_match_rom(bus, rom_code);
    } else {
        onewire_skip_rom(bus);
    }

    onewire_write_byte(bus, command);
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

static bool ds18b20_bus_read_raw(onewire_driver_t* bus,
                                 const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE],
                                 int16_t* raw_temperature)
{
    uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
    uint8_t index = 0U;

    if ((bus == 0) || (raw_temperature == 0) || (rom_code == 0) || !bus->initialized) {
        return false;
    }

    if (!ds18b20_bus_begin_command(bus, rom_code, DS18B20_CMD_READ_SCRATCHPAD)) {
        return false;
    }

    for (index = 0U; index < DS18B20_SCRATCHPAD_SIZE; ++index) {
        scratchpad[index] = onewire_read_byte(bus);
    }

    if (!ds18b20_crc8_is_valid(scratchpad, DS18B20_SCRATCHPAD_SIZE)) {
        return false;
    }

    *raw_temperature = (int16_t) (((uint16_t) scratchpad[1] << 8U) | scratchpad[0]);
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

    driver->use_match_rom = false;
    (void) memset(driver->rom_code, 0, sizeof(driver->rom_code));
    ds18b20_reset_state(driver);
    driver->initialized = ds18b20_is_present(driver);
    return driver->initialized;
}

bool ds18b20_init_with_rom(ds18b20_driver_t* driver,
                           const onewire_pin_config_t* pin_config,
                           const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE])
{
    if ((driver == 0) || (pin_config == 0) || (rom_code == 0)) {
        return false;
    }

    if ((rom_code[0] != DS18B20_FAMILY_CODE)
        || !ds18b20_crc8_is_valid(rom_code, ONEWIRE_ROM_CODE_SIZE)) {
        return false;
    }

    if (!onewire_init(&driver->bus, pin_config)) {
        return false;
    }

    driver->use_match_rom = true;
    (void) memcpy(driver->rom_code, rom_code, sizeof(driver->rom_code));
    ds18b20_reset_state(driver);
    driver->initialized = ds18b20_is_present(driver);
    return driver->initialized;
}

uint8_t ds18b20_discover(const onewire_pin_config_t* pin_config,
                         uint8_t rom_codes[][ONEWIRE_ROM_CODE_SIZE],
                         uint8_t max_devices)
{
    onewire_driver_t bus;
    uint8_t raw_rom_codes[16U][ONEWIRE_ROM_CODE_SIZE];
    uint8_t raw_count = 0U;
    uint8_t valid_count = 0U;
    uint8_t index = 0U;

    if ((pin_config == 0) || (rom_codes == 0) || (max_devices == 0U)) {
        return 0U;
    }

    if (max_devices > 16U) {
        max_devices = 16U;
    }

    if (!onewire_init(&bus, pin_config)) {
        return 0U;
    }

    raw_count = onewire_search_roms(&bus, raw_rom_codes, max_devices);

    for (index = 0U; index < raw_count; ++index) {
        if ((raw_rom_codes[index][0] == DS18B20_FAMILY_CODE)
            && ds18b20_crc8_is_valid(raw_rom_codes[index], ONEWIRE_ROM_CODE_SIZE)) {
            (void) memcpy(rom_codes[valid_count], raw_rom_codes[index], ONEWIRE_ROM_CODE_SIZE);
            valid_count++;
        }
    }

    return valid_count;
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

    return ds18b20_crc8_is_valid(scratchpad, DS18B20_SCRATCHPAD_SIZE);
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

bool ds18b20_bus_init(ds18b20_bus_driver_t* driver, const onewire_pin_config_t* pin_config)
{
    if ((driver == 0) || (pin_config == 0)) {
        return false;
    }

    if (!onewire_init(&driver->bus, pin_config)) {
        return false;
    }

    driver->initialized = true;
    driver->device_count = 0U;
    driver->current_index = 0U;
    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_BUS_STATE_IDLE;
    (void) memset(driver->devices, 0, sizeof(driver->devices));
    return true;
}

uint8_t ds18b20_bus_discover(ds18b20_bus_driver_t* driver)
{
    uint8_t raw_rom_codes[DS18B20_MAX_DEVICES][ONEWIRE_ROM_CODE_SIZE];
    uint8_t raw_count = 0U;
    uint8_t valid_count = 0U;
    uint8_t index = 0U;

    if ((driver == 0) || !driver->initialized) {
        return 0U;
    }

    raw_count = onewire_search_roms(&driver->bus, raw_rom_codes, DS18B20_MAX_DEVICES);
    (void) memset(driver->devices, 0, sizeof(driver->devices));

    for (index = 0U; index < raw_count; ++index) {
        if ((raw_rom_codes[index][0] == DS18B20_FAMILY_CODE)
            && ds18b20_crc8_is_valid(raw_rom_codes[index], ONEWIRE_ROM_CODE_SIZE)) {
            (void) memcpy(driver->devices[valid_count].rom_code,
                          raw_rom_codes[index],
                          ONEWIRE_ROM_CODE_SIZE);
            driver->devices[valid_count].sample_valid = false;
            driver->devices[valid_count].last_raw_temperature = 0;
            valid_count++;
        }
    }

    driver->device_count = valid_count;
    driver->current_index = 0U;
    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_BUS_STATE_IDLE;
    return driver->device_count;
}

bool ds18b20_bus_start_conversion(ds18b20_bus_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized || (driver->device_count == 0U)) {
        return false;
    }

    if (!ds18b20_bus_begin_command(&driver->bus, 0, DS18B20_CMD_CONVERT_T)) {
        driver->state = DS18B20_BUS_STATE_ERROR;
        return false;
    }

    driver->current_index = 0U;
    driver->conversion_elapsed_ms = 0U;
    driver->state = DS18B20_BUS_STATE_CONVERTING;
    return true;
}

void ds18b20_bus_process(ds18b20_bus_driver_t* driver, uint16_t elapsed_ms)
{
    if ((driver == 0) || !driver->initialized || (driver->device_count == 0U)) {
        return;
    }

    switch (driver->state) {
    case DS18B20_BUS_STATE_CONVERTING:
        if ((uint32_t) driver->conversion_elapsed_ms + elapsed_ms >= DS18B20_CONVERSION_TIME_MS) {
            driver->conversion_elapsed_ms = DS18B20_CONVERSION_TIME_MS;
            driver->current_index = 0U;
            driver->state = DS18B20_BUS_STATE_READING;
        } else {
            driver->conversion_elapsed_ms = (uint16_t) (driver->conversion_elapsed_ms + elapsed_ms);
        }
        break;

    case DS18B20_BUS_STATE_READING:
        if (driver->current_index < driver->device_count) {
            int16_t raw_temperature = 0;
            ds18b20_device_t* device = &driver->devices[driver->current_index];

            if (ds18b20_bus_read_raw(&driver->bus, device->rom_code, &raw_temperature)) {
                device->last_raw_temperature = raw_temperature;
                device->sample_valid = true;
            } else {
                device->sample_valid = false;
                driver->state = DS18B20_BUS_STATE_ERROR;
            }

            driver->current_index++;
            if (driver->current_index >= driver->device_count) {
                if (driver->state != DS18B20_BUS_STATE_ERROR) {
                    driver->state = DS18B20_BUS_STATE_IDLE;
                }
            }
        } else {
            driver->state = DS18B20_BUS_STATE_IDLE;
        }
        break;

    case DS18B20_BUS_STATE_IDLE:
    case DS18B20_BUS_STATE_ERROR:
    default:
        break;
    }
}

bool ds18b20_bus_is_busy(const ds18b20_bus_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized) {
        return false;
    }

    return (driver->state == DS18B20_BUS_STATE_CONVERTING)
           || (driver->state == DS18B20_BUS_STATE_READING);
}

uint8_t ds18b20_bus_get_device_count(const ds18b20_bus_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized) {
        return 0U;
    }

    return driver->device_count;
}

bool ds18b20_bus_has_valid_sample(const ds18b20_bus_driver_t* driver, uint8_t index)
{
    if ((driver == 0) || !driver->initialized || (index >= driver->device_count)) {
        return false;
    }

    return driver->devices[index].sample_valid;
}

bool ds18b20_bus_get_latest_raw(const ds18b20_bus_driver_t* driver,
                                uint8_t index,
                                int16_t* raw_temperature)
{
    if ((driver == 0) || (raw_temperature == 0) || !driver->initialized
        || (index >= driver->device_count) || !driver->devices[index].sample_valid) {
        return false;
    }

    *raw_temperature = driver->devices[index].last_raw_temperature;
    return true;
}

bool ds18b20_bus_get_latest_temperature_celsius(const ds18b20_bus_driver_t* driver,
                                                uint8_t index,
                                                float* temperature_celsius)
{
    if ((temperature_celsius == 0) || !ds18b20_bus_has_valid_sample(driver, index)) {
        return false;
    }

    *temperature_celsius = (float) driver->devices[index].last_raw_temperature / 16.0f;
    return true;
}

bool ds18b20_bus_get_rom_code(const ds18b20_bus_driver_t* driver,
                              uint8_t index,
                              uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE])
{
    if ((driver == 0) || (rom_code == 0) || !driver->initialized || (index >= driver->device_count)) {
        return false;
    }

    (void) memcpy(rom_code, driver->devices[index].rom_code, ONEWIRE_ROM_CODE_SIZE);
    return true;
}
