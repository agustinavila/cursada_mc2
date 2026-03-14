/**
 * @file ds18b20_driver.h
 * @brief Interfaz del driver para sensor de temperatura DS18B20
 */

#if !defined(DRIVER_DS18B20_DRIVER_H_)
#define DRIVER_DS18B20_DRIVER_H_

#include "onewire_driver.h"

#include <stdbool.h>
#include <stdint.h>

/** @brief Tamano del scratchpad del DS18B20. */
#define DS18B20_SCRATCHPAD_SIZE 9U

/**
 * @brief Estado del driver DS18B20 para un sensor individual.
 *
 * En esta primera version se asume un unico sensor por bus, por lo que las
 * transacciones usan el comando Skip ROM.
 */
typedef struct {
    onewire_driver_t bus;
    bool initialized;
} ds18b20_driver_t;

/**
 * @brief Inicializa el driver DS18B20 sobre el pin indicado.
 *
 * @param driver Instancia del driver a inicializar.
 * @param pin_config Configuracion fisica del pin del bus 1-Wire.
 *
 * @retval true Si la inicializacion fue correcta y el sensor respondio.
 * @retval false Si hubo un error de parametros o no se detecto presencia.
 */
bool ds18b20_init(ds18b20_driver_t* driver, const onewire_pin_config_t* pin_config);

/**
 * @brief Verifica si hay un dispositivo presente en el bus del sensor.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si hubo pulso de presencia.
 * @retval false En caso contrario.
 */
bool ds18b20_is_present(ds18b20_driver_t* driver);

/**
 * @brief Inicia una conversion de temperatura.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si el comando pudo enviarse.
 * @retval false Si el sensor no respondio.
 */
bool ds18b20_start_conversion(ds18b20_driver_t* driver);

/**
 * @brief Lee el scratchpad completo del sensor.
 *
 * @param driver Instancia del driver.
 * @param scratchpad Buffer destino de 9 bytes.
 *
 * @retval true Si la lectura fue correcta y el CRC valido.
 * @retval false Si hubo error de presencia, parametros o CRC.
 */
bool ds18b20_read_scratchpad(ds18b20_driver_t* driver, uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE]);

/**
 * @brief Lee la temperatura del sensor en grados Celsius.
 *
 * Esta funcion realiza una conversion bloqueante completa.
 *
 * @param driver Instancia del driver.
 * @param temperature_celsius Destino para la temperatura leida.
 *
 * @retval true Si la lectura fue correcta.
 * @retval false Si hubo error de presencia, lectura o CRC.
 */
bool ds18b20_read_temperature_celsius(ds18b20_driver_t* driver, float* temperature_celsius);

/**
 * @brief Lee la temperatura cruda del sensor en formato DS18B20.
 *
 * Esta funcion realiza una conversion bloqueante completa.
 *
 * @param driver Instancia del driver.
 * @param raw_temperature Destino para el valor crudo de 16 bits.
 *
 * @retval true Si la lectura fue correcta.
 * @retval false Si hubo error de presencia, lectura o CRC.
 */
bool ds18b20_read_raw(ds18b20_driver_t* driver, int16_t* raw_temperature);

#endif // DRIVER_DS18B20_DRIVER_H_
