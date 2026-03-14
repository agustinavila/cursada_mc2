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
/** @brief Tiempo maximo de conversion del DS18B20 a 12 bits. */
#define DS18B20_CONVERSION_TIME_MS 750U
/** @brief Codigo de familia 1-Wire del DS18B20. */
#define DS18B20_FAMILY_CODE 0x28U

/**
 * @brief Estado interno de la conversion no bloqueante del sensor.
 */
typedef enum {
    DS18B20_STATE_IDLE = 0,
    DS18B20_STATE_CONVERTING,
    DS18B20_STATE_DATA_READY,
    DS18B20_STATE_ERROR,
} ds18b20_state_t;

/**
 * @brief Estado del driver DS18B20 para un sensor individual.
 *
 * En esta primera version se asume un unico sensor por bus, por lo que las
 * transacciones usan el comando Skip ROM.
 */
typedef struct {
    onewire_driver_t bus;
    bool initialized;
    bool use_match_rom;
    uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE];
    bool sample_valid;
    int16_t last_raw_temperature;
    uint16_t conversion_elapsed_ms;
    ds18b20_state_t state;
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
 * @brief Inicializa un sensor DS18B20 concreto identificado por su ROM.
 *
 * Esta funcion permite trabajar con multiples sensores en el mismo bus
 * 1-Wire, direccionando cada uno mediante Match ROM.
 *
 * @param driver Instancia del driver a inicializar.
 * @param pin_config Configuracion fisica del pin del bus 1-Wire.
 * @param rom_code Codigo ROM del sensor a asociar con la instancia.
 *
 * @retval true Si la inicializacion fue correcta.
 * @retval false Si hubo error de parametros, ROM invalida o el sensor no respondio.
 */
bool ds18b20_init_with_rom(ds18b20_driver_t* driver,
                           const onewire_pin_config_t* pin_config,
                           const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE]);

/**
 * @brief Busca sensores DS18B20 presentes en el bus indicado.
 *
 * @param pin_config Configuracion fisica del pin del bus 1-Wire.
 * @param rom_codes Tabla de salida con los ROMs encontrados.
 * @param max_devices Cantidad maxima de sensores a almacenar.
 *
 * @return Cantidad de sensores DS18B20 encontrados.
 */
uint8_t ds18b20_discover(const onewire_pin_config_t* pin_config,
                         uint8_t rom_codes[][ONEWIRE_ROM_CODE_SIZE],
                         uint8_t max_devices);

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
 * @brief Avanza la maquina de estados no bloqueante del sensor.
 *
 * Debe llamarse periodicamente desde el lazo principal o desde una tarea
 * cooperativa. Cuando la conversion termina, el driver lee el scratchpad y
 * actualiza la ultima muestra valida disponible.
 *
 * @param driver Instancia del driver.
 * @param elapsed_ms Tiempo transcurrido desde la ultima llamada, en ms.
 */
void ds18b20_process(ds18b20_driver_t* driver, uint16_t elapsed_ms);

/**
 * @brief Indica si hay una conversion en curso.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si el sensor esta convirtiendo temperatura.
 * @retval false Si no hay conversion pendiente o el driver es invalido.
 */
bool ds18b20_is_busy(const ds18b20_driver_t* driver);

/**
 * @brief Indica si el driver dispone de una ultima muestra valida.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si existe una temperatura valida cacheada.
 * @retval false En caso contrario.
 */
bool ds18b20_has_valid_sample(const ds18b20_driver_t* driver);

/**
 * @brief Obtiene la ultima temperatura cruda ya convertida.
 *
 * Esta funcion no dispara una nueva conversion. Solo devuelve la ultima
 * muestra valida almacenada por el driver.
 *
 * @param driver Instancia del driver.
 * @param raw_temperature Destino para el valor crudo de 16 bits.
 *
 * @retval true Si habia una muestra valida disponible.
 * @retval false Si no habia datos validos o hubo error de parametros.
 */
bool ds18b20_get_latest_raw(const ds18b20_driver_t* driver, int16_t* raw_temperature);

/**
 * @brief Obtiene la ultima temperatura valida en grados Celsius.
 *
 * Esta funcion no dispara una nueva conversion. Solo convierte la ultima
 * muestra valida almacenada por el driver.
 *
 * @param driver Instancia del driver.
 * @param temperature_celsius Destino para la temperatura en Celsius.
 *
 * @retval true Si habia una muestra valida disponible.
 * @retval false Si no habia datos validos o hubo error de parametros.
 */
bool ds18b20_get_latest_temperature_celsius(const ds18b20_driver_t* driver,
                                            float* temperature_celsius);

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
