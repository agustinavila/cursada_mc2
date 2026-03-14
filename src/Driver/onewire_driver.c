/**
 * @file onewire_driver.c
 * @brief Implementacion del driver para bus 1-Wire por bit-banging
 */

#include "onewire_driver.h"

#include "delay_driver.h"

#define ONEWIRE_CMD_SEARCH_ROM 0xF0U
#define ONEWIRE_CMD_MATCH_ROM  0x55U
#define ONEWIRE_CMD_SKIP_ROM   0xCCU

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

void onewire_skip_rom(onewire_driver_t* driver)
{
    if ((driver == 0) || !driver->initialized) {
        return;
    }

    onewire_write_byte(driver, ONEWIRE_CMD_SKIP_ROM);
}

void onewire_match_rom(onewire_driver_t* driver, const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE])
{
    uint8_t byte_index = 0U;

    if ((driver == 0) || (rom_code == 0) || !driver->initialized) {
        return;
    }

    onewire_write_byte(driver, ONEWIRE_CMD_MATCH_ROM);

    for (byte_index = 0U; byte_index < ONEWIRE_ROM_CODE_SIZE; ++byte_index) {
        onewire_write_byte(driver, rom_code[byte_index]);
    }
}

uint8_t onewire_search_roms(onewire_driver_t* driver,
                            uint8_t rom_codes[][ONEWIRE_ROM_CODE_SIZE],
                            uint8_t max_devices)
{
    uint8_t device_count = 0U;
    uint8_t current_rom[ONEWIRE_ROM_CODE_SIZE] = {0U};
    uint8_t last_discrepancy = 0U;
    bool last_device_flag = false;

    if ((driver == 0) || (rom_codes == 0) || (max_devices == 0U) || !driver->initialized) {
        return 0U;
    }

    while (!last_device_flag && (device_count < max_devices)) {
        uint8_t rom_byte_index = 0U;
        uint8_t rom_bit_mask = 0x01U;
        uint8_t bit_number = 1U;
        uint8_t last_zero = 0U;
        bool search_failed = false;

        if (!onewire_reset(driver)) {
            break;
        }

        onewire_write_byte(driver, ONEWIRE_CMD_SEARCH_ROM);

        while (bit_number <= 64U) {
            const bool id_bit = onewire_read_bit(driver);
            const bool cmp_id_bit = onewire_read_bit(driver);
            bool search_direction = false;

            if (id_bit && cmp_id_bit) {
                search_failed = true;
                break;
            }

            if (id_bit != cmp_id_bit) {
                search_direction = id_bit;
            } else {
                if (bit_number < last_discrepancy) {
                    search_direction = ((current_rom[rom_byte_index] & rom_bit_mask) != 0U);
                } else {
                    search_direction = (bit_number == last_discrepancy);
                }

                if (!search_direction) {
                    last_zero = bit_number;
                }
            }

            if (search_direction) {
                current_rom[rom_byte_index] |= rom_bit_mask;
            } else {
                current_rom[rom_byte_index] &= (uint8_t) ~rom_bit_mask;
            }

            onewire_write_bit(driver, search_direction);

            bit_number++;
            rom_bit_mask <<= 1U;
            if (rom_bit_mask == 0U) {
                rom_byte_index++;
                rom_bit_mask = 0x01U;
            }
        }

        if (search_failed || (bit_number <= 64U)) {
            break;
        }

        for (rom_byte_index = 0U; rom_byte_index < ONEWIRE_ROM_CODE_SIZE; ++rom_byte_index) {
            rom_codes[device_count][rom_byte_index] = current_rom[rom_byte_index];
        }

        device_count++;
        last_discrepancy = last_zero;
        if (last_discrepancy == 0U) {
            last_device_flag = true;
        }
    }

    return device_count;
}
