#include "fake_eeprom_driver.h"

#include "drivers/eeprom_driver.h"

#include <string.h>

static uint8_t fake_eeprom_region_[DRIVER_EEPROM_REGION_SIZE];
static bool fake_eeprom_read_ok_ = true;
static bool fake_eeprom_write_ok_ = true;

static bool fake_eeprom_rango_valido(uint32_t offset, uint32_t cantidad)
{
    if (cantidad > DRIVER_EEPROM_REGION_SIZE) {
        return false;
    }

    return offset <= (DRIVER_EEPROM_REGION_SIZE - cantidad);
}

void fake_eeprom_reset(void)
{
    (void) memset(fake_eeprom_region_, 0, sizeof(fake_eeprom_region_));
    fake_eeprom_read_ok_ = true;
    fake_eeprom_write_ok_ = true;
}

void fake_eeprom_set_read_ok(bool habilitado)
{
    fake_eeprom_read_ok_ = habilitado;
}

void fake_eeprom_set_write_ok(bool habilitado)
{
    fake_eeprom_write_ok_ = habilitado;
}

void fake_eeprom_corrupt_byte(uint32_t offset, uint8_t valor)
{
    if (offset < DRIVER_EEPROM_REGION_SIZE) {
        fake_eeprom_region_[offset] = valor;
    }
}

bool fake_eeprom_read_back(uint32_t offset, void* destino, uint32_t cantidad)
{
    if ((destino == 0) || !fake_eeprom_rango_valido(offset, cantidad)) {
        return false;
    }

    (void) memcpy(destino, &fake_eeprom_region_[offset], cantidad);
    return true;
}

bool driver_eeprom_init(void)
{
    return true;
}

bool driver_eeprom_read(uint32_t offset, void* destino, uint32_t cantidad)
{
    if (!fake_eeprom_read_ok_ || (destino == 0) || !fake_eeprom_rango_valido(offset, cantidad)) {
        return false;
    }

    (void) memcpy(destino, &fake_eeprom_region_[offset], cantidad);
    return true;
}

bool driver_eeprom_write(uint32_t offset, const void* origen, uint32_t cantidad)
{
    if (!fake_eeprom_write_ok_ || (origen == 0) || !fake_eeprom_rango_valido(offset, cantidad)) {
        return false;
    }

    (void) memcpy(&fake_eeprom_region_[offset], origen, cantidad);
    return true;
}
