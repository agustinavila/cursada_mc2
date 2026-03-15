/**
 * @file eeprom_driver.h
 * @brief Driver de acceso a la EEPROM interna del LPC4337.
 */

#if !defined(EEPROM_DRIVER_H_)
#define EEPROM_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

/** Region reservada para persistencia de parametros. */
#define DRIVER_EEPROM_REGION_OFFSET 0U
/** Tamano reservado para datos persistentes de la aplicacion. */
#define DRIVER_EEPROM_REGION_SIZE   256U

/**
 * @brief Inicializa el periferico EEPROM.
 *
 * @retval true Si la EEPROM quedo lista para usarse.
 * @retval false Si no pudo inicializarse.
 */
bool driver_eeprom_init(void);

/**
 * @brief Lee datos desde la region reservada de EEPROM.
 *
 * @param offset Offset logico dentro de la region reservada.
 * @param destino Buffer de destino.
 * @param cantidad Cantidad de bytes a leer.
 *
 * @retval true Si la lectura fue valida.
 * @retval false Si el rango o los punteros son invalidos.
 */
bool driver_eeprom_read(uint32_t offset, void* destino, uint32_t cantidad);

/**
 * @brief Escribe datos dentro de la region reservada de EEPROM.
 *
 * @param offset Offset logico dentro de la region reservada.
 * @param origen Buffer de origen.
 * @param cantidad Cantidad de bytes a escribir.
 *
 * @retval true Si la escritura fue valida.
 * @retval false Si el rango o los punteros son invalidos.
 */
bool driver_eeprom_write(uint32_t offset, const void* origen, uint32_t cantidad);

#endif // EEPROM_DRIVER_H_
