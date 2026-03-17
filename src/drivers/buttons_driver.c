/**
 * @file teclas_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "buttons_driver.h"

#include <stdbool.h>
#include <stddef.h>

#define BUTTONS_CANTIDAD 4U
#define BUTTONS_DEBOUNCE_MS 40U

typedef struct {
    bool irq_pendiente;
    bool presionado_estable;
    bool armado;
    bool evento_pendiente;
    uint32_t debounce_acumulado_ms;
} button_estado_t;

static button_estado_t button_estados_[BUTTONS_CANTIDAD] = {
    [0] = {.armado = true},
    [1] = {.armado = true},
    [2] = {.armado = true},
    [3] = {.armado = true},
};

static uint8_t button_indice_desde_tecla(uint8_t tecla)
{
    switch (tecla) {
    case TECLA1:
        return 0U;
    case TECLA2:
        return 1U;
    case TECLA3:
        return 2U;
    case TECLA4:
        return 3U;
    default:
        return 0xFFU;
    }
}

void buttons_init(void)
{
    uint8_t indice = 0U;

    Chip_GPIO_Init(LPC_GPIO_PORT);
    Chip_SCU_PinMux(1, 0, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 1, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 2, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 6, MD_PUP | MD_EZI | MD_ZI, FUNC0);

    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 4), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 8), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 9), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 9), 0);

    for (indice = 0U; indice < BUTTONS_CANTIDAD; indice++) {
        button_estados_[indice].irq_pendiente = false;
        button_estados_[indice].presionado_estable = false;
        button_estados_[indice].armado = true;
        button_estados_[indice].evento_pendiente = false;
        button_estados_[indice].debounce_acumulado_ms = 0U;
    }

    button_int_enable(TECLA1);
    button_int_enable(TECLA2);
    button_int_enable(TECLA3);
    button_int_enable(TECLA4);
}

uint8_t button_read_pin(uint8_t numero_tecla)
{
    uint8_t tecla = 0;
    switch (numero_tecla) {
    case TECLA1:
        tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 4));
        break;
    case TECLA2:
        tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 8));
        break;
    case TECLA3:
        tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 9));
        break;
    case TECLA4:
        tecla = (0x01) & (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, 1, 9));
        break;
    default:
        tecla = 0;
        break;
    }
    return tecla;
}

uint8_t button_read_all_pins(void)
{
    uint8_t teclas = 0;
    teclas = button_read_pin(TECLA1);
    teclas |= (button_read_pin(TECLA2) << 1);
    teclas |= (button_read_pin(TECLA3) << 2);
    teclas |= (button_read_pin(TECLA4) << 3);
    return teclas;
}


void button_int_enable(uint8_t tecla)
{
    switch (tecla) {
    case TECLA1:
        Chip_SCU_GPIOIntPinSel(0, 0, 4);
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);
        Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);

        NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
        NVIC_EnableIRQ(PIN_INT0_IRQn);
        break;

    case TECLA2:
        Chip_SCU_GPIOIntPinSel(1, 0, 8);
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1);
        Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);

        NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
        NVIC_EnableIRQ(PIN_INT1_IRQn);
        break;

    case TECLA3:
        Chip_SCU_GPIOIntPinSel(2, 0, 9);
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);
        Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);

        NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
        NVIC_EnableIRQ(PIN_INT2_IRQn);
        break;

    case TECLA4:
        Chip_SCU_GPIOIntPinSel(3, 1, 9);
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH3);
        Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);

        NVIC_ClearPendingIRQ(PIN_INT3_IRQn);
        NVIC_EnableIRQ(PIN_INT3_IRQn);
        break;

    default:
        break;
    }
}

void button_notify_irq(uint8_t button_id)
{
    const uint8_t indice = button_indice_desde_tecla(button_id);

    if (indice >= BUTTONS_CANTIDAD) {
        return;
    }

    button_estados_[indice].irq_pendiente = true;
    button_estados_[indice].debounce_acumulado_ms = 0U;
}

void buttons_process(uint32_t delta_ms)
{
    uint8_t indice = 0U;

    for (indice = 0U; indice < BUTTONS_CANTIDAD; indice++) {
        const uint8_t tecla = (uint8_t) (indice + 1U);
        const bool presionado_actual = (button_read_pin(tecla) != 0U);
        button_estado_t* estado = &button_estados_[indice];

        if (estado->presionado_estable && !presionado_actual) {
            estado->presionado_estable = false;
            estado->armado = true;
        }

        if (!estado->irq_pendiente) {
            continue;
        }

        if (!presionado_actual) {
            estado->irq_pendiente = false;
            estado->debounce_acumulado_ms = 0U;
            continue;
        }

        if (estado->debounce_acumulado_ms < BUTTONS_DEBOUNCE_MS) {
            const uint32_t tiempo_restante = BUTTONS_DEBOUNCE_MS - estado->debounce_acumulado_ms;
            estado->debounce_acumulado_ms += (delta_ms < tiempo_restante) ? delta_ms : tiempo_restante;
        }

        if (estado->debounce_acumulado_ms < BUTTONS_DEBOUNCE_MS) {
            continue;
        }

        estado->irq_pendiente = false;
        estado->debounce_acumulado_ms = 0U;

        if (estado->armado) {
            estado->presionado_estable = true;
            estado->armado = false;
            estado->evento_pendiente = true;
        }
    }
}

uint8_t button_get_event(void)
{
    uint8_t indice = 0U;

    for (indice = 0U; indice < BUTTONS_CANTIDAD; indice++) {
        if (!button_estados_[indice].evento_pendiente) {
            continue;
        }

        button_estados_[indice].evento_pendiente = false;
        return (uint8_t) (indice + 1U);
    }

    return 0U;
}
