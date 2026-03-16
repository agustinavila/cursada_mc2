#if !defined(EEPROM_DRIVER_H_)
#define EEPROM_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

#define DRIVER_EEPROM_REGION_OFFSET 0U
#define DRIVER_EEPROM_REGION_SIZE   256U

bool driver_eeprom_init(void);
bool driver_eeprom_read(uint32_t offset, void* destino, uint32_t cantidad);
bool driver_eeprom_write(uint32_t offset, const void* origen, uint32_t cantidad);

#endif // EEPROM_DRIVER_H_
