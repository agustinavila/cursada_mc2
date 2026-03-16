#if !defined(TESTS_SUPPORT_FAKE_EEPROM_DRIVER_H_)
#define TESTS_SUPPORT_FAKE_EEPROM_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

void fake_eeprom_reset(void);
void fake_eeprom_set_read_ok(bool habilitado);
void fake_eeprom_set_write_ok(bool habilitado);
void fake_eeprom_corrupt_byte(uint32_t offset, uint8_t valor);
bool fake_eeprom_read_back(uint32_t offset, void* destino, uint32_t cantidad);

#endif // TESTS_SUPPORT_FAKE_EEPROM_DRIVER_H_
