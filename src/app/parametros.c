/**
 * @file parametros.c
 * @brief Persistencia y administracion de parametros de la aplicacion.
 */

#include "app/parametros.h"

#include "Driver/eeprom_driver.h"
#include "app/parametros_default.h"

#include <string.h>

#define PARAMETROS_PERSISTENTES_MAGIC   0x5041524DU
#define PARAMETROS_PERSISTENTES_VERSION 2U

/**
 * @brief Formato persistido en EEPROM para los parametros de la aplicacion.
 *
 * La cabecera permite validar compatibilidad basica y detectar corrupcion.
 * Si alguno de estos campos no coincide al arrancar, se descarta el contenido
 * persistido y se vuelven a cargar los valores por defecto.
 */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t tamano;
    uint32_t crc;
    parametros_t datos;
} parametros_persistentes_t;

static parametros_t parametros_actuales_;

/**
 * @brief Calcula el CRC32 del bloque persistido de parametros.
 *
 * El CRC solo cubre los datos utiles, no la cabecera, para poder recalcularlo
 * luego de serializar los campos de control.
 */
static uint32_t parametros_calcular_crc32(const void* datos, uint32_t longitud)
{
    const uint8_t* bytes = (const uint8_t*) datos;
    uint32_t crc = 0xFFFFFFFFU;
    uint32_t indice = 0U;

    for (indice = 0U; indice < longitud; ++indice) {
        crc ^= bytes[indice];
        for (uint32_t bit = 0U; bit < 8U; ++bit) {
            if ((crc & 1U) != 0U) {
                crc = (crc >> 1U) ^ 0xEDB88320U;
            } else {
                crc >>= 1U;
            }
        }
    }

    return ~crc;
}

static void parametros_cargar_defaults_en_ram(void)
{
    parametros_actuales_ = *parametros_default_obtener();
}

/**
 * @brief Genera la imagen persistida a partir de la copia viva en RAM.
 */
static void parametros_serializar(parametros_persistentes_t* persistentes)
{
    persistentes->magic = PARAMETROS_PERSISTENTES_MAGIC;
    persistentes->version = PARAMETROS_PERSISTENTES_VERSION;
    persistentes->tamano = (uint16_t) sizeof(parametros_t);
    persistentes->datos = parametros_actuales_;
    persistentes->crc = parametros_calcular_crc32(&persistentes->datos, (uint32_t) sizeof(persistentes->datos));
}

/**
 * @brief Valida magic, version, tamano y CRC antes de aceptar EEPROM.
 */
static bool parametros_persistentes_validos(const parametros_persistentes_t* persistentes)
{
    if (persistentes->magic != PARAMETROS_PERSISTENTES_MAGIC) {
        return false;
    }

    if (persistentes->version != PARAMETROS_PERSISTENTES_VERSION) {
        return false;
    }

    if (persistentes->tamano != sizeof(parametros_t)) {
        return false;
    }

    return (persistentes->crc
        == parametros_calcular_crc32(&persistentes->datos, (uint32_t) sizeof(persistentes->datos)));
}

bool parametros_init(void)
{
    parametros_persistentes_t persistentes;

    if (!driver_eeprom_read(0U, &persistentes, (uint32_t) sizeof(persistentes))) {
        /* Si no se puede leer EEPROM, se arranca con defaults y se intenta persistirlos. */
        parametros_cargar_defaults_en_ram();
        return parametros_guardar();
    }

    if (!parametros_persistentes_validos(&persistentes)) {
        /* Datos incompatibles o corruptos: se descartan y se regeneran desde defaults. */
        parametros_cargar_defaults_en_ram();
        return parametros_guardar();
    }

    parametros_actuales_ = persistentes.datos;
    return true;
}

const parametros_t* parametros_obtener(void)
{
    return &parametros_actuales_;
}

bool parametros_guardar(void)
{
    parametros_persistentes_t persistentes;

    parametros_serializar(&persistentes);
    return driver_eeprom_write(0U, &persistentes, (uint32_t) sizeof(persistentes));
}

void parametros_restablecer_defaults(void)
{
    /* Restablecer implica actualizar la copia RAM y persistirla inmediatamente. */
    parametros_cargar_defaults_en_ram();
    (void) parametros_guardar();
}

bool parametros_actualizar_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   bool modo_calentar)
{
    bool hubo_cambios = false;

    if (parametros_actuales_.control.setpoint_deci_celsius != setpoint_deci_celsius) {
        parametros_actuales_.control.setpoint_deci_celsius = setpoint_deci_celsius;
        hubo_cambios = true;
    }

    if (parametros_actuales_.control.histeresis_deci_celsius != histeresis_deci_celsius) {
        parametros_actuales_.control.histeresis_deci_celsius = histeresis_deci_celsius;
        hubo_cambios = true;
    }

    if (parametros_actuales_.control.modo_calentar != modo_calentar) {
        parametros_actuales_.control.modo_calentar = modo_calentar;
        hubo_cambios = true;
    }

    return hubo_cambios;
}

bool parametros_configurar_sensor_proceso_auto(void)
{
    bool hubo_cambios = false;
    uint32_t indice = 0U;

    if (parametros_actuales_.sensor_proceso.modo != PARAMETROS_SENSOR_MODO_AUTO) {
        parametros_actuales_.sensor_proceso.modo = PARAMETROS_SENSOR_MODO_AUTO;
        hubo_cambios = true;
    }

    if (parametros_actuales_.sensor_proceso.rom_valida) {
        parametros_actuales_.sensor_proceso.rom_valida = false;
        hubo_cambios = true;
    }

    for (indice = 0U; indice < PARAMETROS_SENSOR_ROM_SIZE; ++indice) {
        if (parametros_actuales_.sensor_proceso.rom[indice] != 0U) {
            parametros_actuales_.sensor_proceso.rom[indice] = 0U;
            hubo_cambios = true;
        }
    }

    return hubo_cambios;
}

bool parametros_configurar_sensor_proceso_por_rom(const uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE])
{
    bool hubo_cambios = false;

    if (rom == 0) {
        return false;
    }

    if (parametros_actuales_.sensor_proceso.modo != PARAMETROS_SENSOR_MODO_ROM) {
        parametros_actuales_.sensor_proceso.modo = PARAMETROS_SENSOR_MODO_ROM;
        hubo_cambios = true;
    }

    if (!parametros_actuales_.sensor_proceso.rom_valida
        || (memcmp(parametros_actuales_.sensor_proceso.rom, rom, PARAMETROS_SENSOR_ROM_SIZE) != 0)) {
        (void) memcpy(parametros_actuales_.sensor_proceso.rom, rom, PARAMETROS_SENSOR_ROM_SIZE);
        parametros_actuales_.sensor_proceso.rom_valida = true;
        hubo_cambios = true;
    }

    return hubo_cambios;
}
