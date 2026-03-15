/**
 * @file eeprom_driver.c
 * @brief Driver de acceso a la EEPROM interna del LPC4337.
 */

#include "Driver/eeprom_driver.h"

#include "chip.h"

#include <string.h>

#define EEPROM_DRIVER_PAGE_WORDS (EEPROM_PAGE_SIZE / sizeof(uint32_t))

static bool eeprom_driver_inicializado_ = false;

/**
 * @brief Verifica que una operacion quede dentro de la region reservada.
 *
 * El driver expone offsets logicos desde cero para no propagar detalles de
 * direccionamiento fisico al resto de la aplicacion.
 */
static bool driver_eeprom_rango_valido(uint32_t offset, uint32_t cantidad)
{
    if (offset > DRIVER_EEPROM_REGION_SIZE) {
        return false;
    }

    if (cantidad > (DRIVER_EEPROM_REGION_SIZE - offset)) {
        return false;
    }

    return true;
}

static uint8_t* driver_eeprom_obtener_direccion(uint32_t offset)
{
    return (uint8_t*) (EEPROM_START + DRIVER_EEPROM_REGION_OFFSET + offset);
}

/**
 * @brief Programa una pagina completa de EEPROM a partir de un buffer RAM.
 *
 * La EEPROM interna del LPC43xx se programa por palabras dentro de una pagina.
 * El driver arma previamente la pagina completa en RAM para poder soportar
 * escrituras parciales sin corromper los bytes vecinos.
 */
static void driver_eeprom_programar_pagina(uint32_t pagina, const uint8_t* datos_pagina)
{
    volatile uint32_t* palabra_destino = (volatile uint32_t*) EEPROM_ADDRESS(pagina, 0U);
    uint32_t palabras_origen[EEPROM_DRIVER_PAGE_WORDS];
    uint32_t indice = 0U;

    (void) memcpy(palabras_origen, datos_pagina, EEPROM_PAGE_SIZE);

    Chip_EEPROM_SetAutoProg(LPC_EEPROM, EEPROM_AUTOPROG_AFT_LASTWORDWRITTEN);
    for (indice = 0U; indice < EEPROM_DRIVER_PAGE_WORDS; ++indice) {
        palabra_destino[indice] = palabras_origen[indice];
    }
    Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
    Chip_EEPROM_SetAutoProg(LPC_EEPROM, EEPROM_AUTOPROG_OFF);
}

bool driver_eeprom_init(void)
{
    Chip_EEPROM_Init(LPC_EEPROM);
    Chip_EEPROM_SetAutoProg(LPC_EEPROM, EEPROM_AUTOPROG_OFF);
    eeprom_driver_inicializado_ = true;
    return true;
}

bool driver_eeprom_read(uint32_t offset, void* destino, uint32_t cantidad)
{
    if (!eeprom_driver_inicializado_ || (destino == 0)) {
        return false;
    }

    if (cantidad == 0U) {
        return true;
    }

    if (!driver_eeprom_rango_valido(offset, cantidad)) {
        return false;
    }

    (void) memcpy(destino, driver_eeprom_obtener_direccion(offset), cantidad);
    return true;
}

bool driver_eeprom_write(uint32_t offset, const void* origen, uint32_t cantidad)
{
    const uint8_t* datos_origen = (const uint8_t*) origen;
    uint8_t pagina_buffer[EEPROM_PAGE_SIZE];

    if (!eeprom_driver_inicializado_ || (origen == 0)) {
        return false;
    }

    if (cantidad == 0U) {
        return true;
    }

    if (!driver_eeprom_rango_valido(offset, cantidad)) {
        return false;
    }

    /**
     * @brief Cada pagina se trata con una estrategia read-modify-write.
     *
     * Primero se copia la pagina actual desde EEPROM, luego se parchea en RAM
     * solo el rango solicitado y finalmente se reprograma la pagina completa
     * solo si hubo cambios reales. Esto evita escrituras innecesarias y permite
     * ofrecer una API por bytes sobre un hardware orientado a paginas.
     */
    while (cantidad > 0U) {
        const uint32_t direccion_actual = DRIVER_EEPROM_REGION_OFFSET + offset;
        const uint32_t pagina = direccion_actual / EEPROM_PAGE_SIZE;
        const uint32_t offset_pagina = direccion_actual % EEPROM_PAGE_SIZE;
        const uint32_t bytes_pagina = EEPROM_PAGE_SIZE - offset_pagina;
        const uint32_t bytes_a_copiar = (cantidad < bytes_pagina) ? cantidad : bytes_pagina;
        const uint8_t* direccion_pagina = (const uint8_t*) EEPROM_ADDRESS(pagina, 0U);

        (void) memcpy(pagina_buffer, direccion_pagina, EEPROM_PAGE_SIZE);
        (void) memcpy(&pagina_buffer[offset_pagina], datos_origen, bytes_a_copiar);

        if (memcmp(pagina_buffer, direccion_pagina, EEPROM_PAGE_SIZE) != 0) {
            driver_eeprom_programar_pagina(pagina, pagina_buffer);
        }

        offset += bytes_a_copiar;
        datos_origen += bytes_a_copiar;
        cantidad -= bytes_a_copiar;
    }

    return true;
}
