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

void buttons_init(void)
{
    Chip_GPIO_Init(LPC_GPIO_PORT);
    Chip_SCU_PinMux(1, 0, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 1, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 2, MD_PUP | MD_EZI | MD_ZI, FUNC0);
    Chip_SCU_PinMux(1, 6, MD_PUP | MD_EZI | MD_ZI, FUNC0);

    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 4), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 8), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 9), 0);
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 9), 0);
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
