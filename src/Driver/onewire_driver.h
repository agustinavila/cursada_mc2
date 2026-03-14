/**
 * @file onewire_driver.h
 * @brief Interfaz del driver para bus 1-Wire por bit-banging
 */

#if !defined(DRIVER_ONEWIRE_DRIVER_H_)
#define DRIVER_ONEWIRE_DRIVER_H_

#include <chip.h>
#include <stdbool.h>
#include <stdint.h>

/** @brief Tamano del codigo ROM de un dispositivo 1-Wire. */
#define ONEWIRE_ROM_CODE_SIZE 8U

/**
 * @brief Configuracion fisica del pin usado por el bus 1-Wire.
 *
 * El bus se maneja como salida open-drain por software:
 * - para escribir '0', el pin se fuerza a salida en bajo
 * - para liberar la linea, el pin se deja como entrada
 *
 * Se asume una resistencia pull-up externa en el bus.
 */
typedef struct {
    uint8_t scu_port;
    uint8_t scu_pin;
    uint16_t scu_mode;
    uint8_t scu_func;
    uint8_t gpio_port;
    uint8_t gpio_pin;
} onewire_pin_config_t;

/**
 * @brief Estado del driver 1-Wire para un bus individual.
 */
typedef struct {
    onewire_pin_config_t pin;
    bool initialized;
} onewire_driver_t;

/**
 * @brief Inicializa un bus 1-Wire sobre el pin indicado.
 *
 * @param driver Instancia del driver a inicializar.
 * @param pin_config Configuracion fisica del pin del bus.
 *
 * @retval true Si la inicializacion fue correcta.
 * @retval false Si algun puntero es invalido.
 */
bool onewire_init(onewire_driver_t* driver, const onewire_pin_config_t* pin_config);

/**
 * @brief Emite un reset 1-Wire y detecta presencia de dispositivos.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si algun dispositivo respondio con pulso de presencia.
 * @retval false Si no hubo presencia o el driver no esta inicializado.
 */
bool onewire_reset(const onewire_driver_t* driver);

/**
 * @brief Escribe un bit en el bus 1-Wire.
 *
 * @param driver Instancia del driver.
 * @param bit_value Valor del bit a transmitir.
 */
void onewire_write_bit(const onewire_driver_t* driver, bool bit_value);

/**
 * @brief Lee un bit desde el bus 1-Wire.
 *
 * @param driver Instancia del driver.
 *
 * @retval true Si el bit leido fue '1'.
 * @retval false Si el bit leido fue '0' o el driver no esta inicializado.
 */
bool onewire_read_bit(const onewire_driver_t* driver);

/**
 * @brief Escribe un byte en el bus 1-Wire.
 *
 * @param driver Instancia del driver.
 * @param value Byte a transmitir, comenzando por el bit menos significativo.
 */
void onewire_write_byte(const onewire_driver_t* driver, uint8_t value);

/**
 * @brief Lee un byte desde el bus 1-Wire.
 *
 * @param driver Instancia del driver.
 *
 * @return Byte leido, comenzando por el bit menos significativo.
 */
uint8_t onewire_read_byte(const onewire_driver_t* driver);

/**
 * @brief Emite el comando Skip ROM sobre el bus.
 *
 * Debe usarse cuando hay un unico dispositivo en el bus, o cuando se desea
 * enviar un comando global a todos los dispositivos.
 *
 * @param driver Instancia del driver.
 */
void onewire_skip_rom(const onewire_driver_t* driver);

/**
 * @brief Selecciona un dispositivo concreto por su codigo ROM.
 *
 * @param driver Instancia del driver.
 * @param rom_code Codigo ROM de 64 bits del dispositivo a seleccionar.
 */
void onewire_match_rom(const onewire_driver_t* driver, const uint8_t rom_code[ONEWIRE_ROM_CODE_SIZE]);

/**
 * @brief Busca dispositivos presentes en el bus mediante el comando Search ROM.
 *
 * @param driver Instancia del driver del bus.
 * @param rom_codes Tabla de salida con los ROM encontrados.
 * @param max_devices Cantidad maxima de dispositivos a almacenar.
 *
 * @return Cantidad de dispositivos encontrados y almacenados.
 */
uint8_t onewire_search_roms(const onewire_driver_t* driver,
                            uint8_t rom_codes[][ONEWIRE_ROM_CODE_SIZE],
                            uint8_t max_devices);

#endif // DRIVER_ONEWIRE_DRIVER_H_
