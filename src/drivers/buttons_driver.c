/**
 * @file buttons_driver.c
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
    uint8_t tecla;
    uint8_t scu_port;
    uint8_t scu_pin;
    uint16_t scu_mode;
    uint8_t scu_func;
    uint8_t gpio_port;
    uint8_t gpio_pin;
    uint8_t pinint_selector;
    uint32_t pinint_mask;
    IRQn_Type irqn;
} button_hw_t;

typedef struct {
    bool irq_pendiente;
    bool armado;
    bool evento_pendiente;
    uint32_t debounce_acumulado_ms;
} button_estado_t;

/* Tabla fija de relacion entre tecla logica y recursos fisicos del LPC4337. */
static const button_hw_t button_hw_[BUTTONS_CANTIDAD] = {
    [0] = {.tecla = TECLA1, .scu_port = 1U, .scu_pin = 0U, .scu_mode = (MD_PUP | MD_EZI | MD_ZI), .scu_func = FUNC0, .gpio_port = 0U, .gpio_pin = 4U, .pinint_selector = 0U, .pinint_mask = PININTCH0, .irqn = PIN_INT0_IRQn},
    [1] = {.tecla = TECLA2, .scu_port = 1U, .scu_pin = 1U, .scu_mode = (MD_PUP | MD_EZI | MD_ZI), .scu_func = FUNC0, .gpio_port = 0U, .gpio_pin = 8U, .pinint_selector = 1U, .pinint_mask = PININTCH1, .irqn = PIN_INT1_IRQn},
    [2] = {.tecla = TECLA3, .scu_port = 1U, .scu_pin = 2U, .scu_mode = (MD_PUP | MD_EZI | MD_ZI), .scu_func = FUNC0, .gpio_port = 0U, .gpio_pin = 9U, .pinint_selector = 2U, .pinint_mask = PININTCH2, .irqn = PIN_INT2_IRQn},
    [3] = {.tecla = TECLA4, .scu_port = 1U, .scu_pin = 6U, .scu_mode = (MD_PUP | MD_EZI | MD_ZI), .scu_func = FUNC0, .gpio_port = 1U, .gpio_pin = 9U, .pinint_selector = 3U, .pinint_mask = PININTCH3, .irqn = PIN_INT3_IRQn},
};

static volatile button_estado_t button_estados_[BUTTONS_CANTIDAD] = {
    [0] = {.armado = true},
    [1] = {.armado = true},
    [2] = {.armado = true},
    [3] = {.armado = true},
};

static int8_t button_indice_desde_tecla(uint8_t tecla)
{
    switch (tecla) {
    case TECLA1:
        return 0;
    case TECLA2:
        return 1;
    case TECLA3:
        return 2;
    case TECLA4:
        return 3;
    default:
        return -1;
    }
}

static bool button_esta_presionado(uint8_t indice)
{
    return !Chip_GPIO_GetPinState(LPC_GPIO_PORT, button_hw_[indice].gpio_port, button_hw_[indice].gpio_pin);
}

void buttons_init(void)
{
    uint8_t indice = 0U;

    Chip_GPIO_Init(LPC_GPIO_PORT);

    for (indice = 0U; indice < BUTTONS_CANTIDAD; indice++) {
        button_estados_[indice].irq_pendiente = false;
        button_estados_[indice].armado = true;
        button_estados_[indice].evento_pendiente = false;
        button_estados_[indice].debounce_acumulado_ms = 0U;

        Chip_SCU_PinMux(button_hw_[indice].scu_port,
                        button_hw_[indice].scu_pin,
                        button_hw_[indice].scu_mode,
                        button_hw_[indice].scu_func);
        Chip_GPIO_SetDir(LPC_GPIO_PORT,
                         button_hw_[indice].gpio_port,
                         (1UL << button_hw_[indice].gpio_pin),
                         0);
        Chip_SCU_GPIOIntPinSel(button_hw_[indice].pinint_selector,
                               button_hw_[indice].gpio_port,
                               button_hw_[indice].gpio_pin);
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, button_hw_[indice].pinint_mask);
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, button_hw_[indice].pinint_mask);
        Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, button_hw_[indice].pinint_mask);
        NVIC_ClearPendingIRQ(button_hw_[indice].irqn);
        NVIC_EnableIRQ(button_hw_[indice].irqn);
    }
}

void button_notify_irq(uint8_t button_id)
{
    const int8_t indice = button_indice_desde_tecla(button_id);

    if (indice < 0) {
        return;
    }

    /* La ISR solo marca actividad y reinicia la ventana de debounce. */
    button_estados_[(uint8_t) indice].irq_pendiente = true;
    button_estados_[(uint8_t) indice].debounce_acumulado_ms = 0U;
}

void buttons_process(uint32_t delta_ms)
{
    uint8_t indice = 0U;

    for (indice = 0U; indice < BUTTONS_CANTIDAD; indice++) {
        const bool presionado_actual = button_esta_presionado(indice);
        volatile button_estado_t* estado = &button_estados_[indice];

        /* Cuando la tecla se libera, se rearma para aceptar una nueva pulsacion. */
        if (estado->armado == false && !presionado_actual) {
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

        /* Solo se genera un evento por pulsacion hasta que la tecla se suelte. */
        if (estado->armado) {
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
        return button_hw_[indice].tecla;
    }

    return 0U;
}
