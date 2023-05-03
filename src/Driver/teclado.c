/**
 * @file teclado.c
 * @author Agustin Avila (tinto.avila@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-04-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "teclado.h"

#include <string.h>


#define KEYBOARD_MAX_ROWS 4
#define KEYBOARD_MAX_COLUMNS 3

#define FILA0_IRQ_HANDLER GPIO4_IRQHandler
#define FILA1_IRQ_HANDLER GPIO5_IRQHandler
#define FILA2_IRQ_HANDLER GPIO6_IRQHandler
#define FILA3_IRQ_HANDLER GPIO7_IRQHandler

uint8_t state_matrix[KEYBOARD_MAX_ROWS][KEYBOARD_MAX_COLUMNS];

uint8_t last_char = 0xFF;


void board_keyboard_init(void)
{
    // Inputs
    Chip_GPIO_Init(LPC_GPIO_PORT);
    Chip_SCU_PinMux(4, 0, MD_PDN | MD_EZI, FUNC0); //< Fila0
    Chip_SCU_PinMux(4, 1, MD_PDN | MD_EZI, FUNC0); //< Fila1
    Chip_SCU_PinMux(4, 2, MD_PDN | MD_EZI, FUNC0); //< Fila2
    Chip_SCU_PinMux(4, 3, MD_PDN | MD_EZI, FUNC0); //< Fila3

    Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 0), 0); //< Fila0
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 1), 0); //< Fila1
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 2), 0); //< Fila2
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1 << 3), 0); //< Fila3

    // Outputs
    Chip_SCU_PinMux(1, 5, MD_PLN, FUNC0); //< Col0
    Chip_SCU_PinMux(7, 4, MD_PLN, FUNC0); //< Col1
    Chip_SCU_PinMux(7, 5, MD_PLN, FUNC0); //< Col2

    Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 8), 1);  //< Col0
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 12), 1); //< Col1
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 13), 1); //< Col2

    // Set outputs to high
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 8);
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 12);
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 13);
}


void board_keyboard_int_enable(void)
{
    Chip_SCU_GPIOIntPinSel(4, 2, 0);
    Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH4);
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH4);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH4);
    NVIC_ClearPendingIRQ(PIN_INT4_IRQn);
    NVIC_EnableIRQ(PIN_INT4_IRQn);

    Chip_SCU_GPIOIntPinSel(5, 2, 1);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH5);
    Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH5);
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH5);
    NVIC_ClearPendingIRQ(PIN_INT5_IRQn);
    NVIC_EnableIRQ(PIN_INT5_IRQn);

    Chip_SCU_GPIOIntPinSel(6, 2, 2);
    Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH6);
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH6);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH6);
    NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
    NVIC_EnableIRQ(PIN_INT6_IRQn);

    Chip_SCU_GPIOIntPinSel(7, 2, 3);
    Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH7);
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH7);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH7);
    NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
    NVIC_EnableIRQ(PIN_INT7_IRQn);
}


int board_keyboard_read_cell(uint8_t fila, uint8_t columna) { return state_matrix[fila][columna]; }


void board_keyboard_read_matrix(uint8_t* matrix_p[KEYBOARD_MAX_COLUMNS])
{
    memcpy((char*) matrix_p, (char*) state_matrix, sizeof(uint8_t) * KEYBOARD_MAX_COLUMNS * KEYBOARD_MAX_ROWS);
    for (int row = 0; row < KEYBOARD_MAX_ROWS; row++)
    {
        for (int col = 0; col < KEYBOARD_MAX_COLUMNS; col++) { state_matrix[row][col] = 0; }
    }
}


char board_keyboard_get_last_char()
{
    uint8_t lchar = last_char;
    last_char = 0xFF;
    return lchar;
}


uint8_t read_row(uint8_t row)
{
    uint8_t col = 0;
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 12);
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 13);
    if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row))
    {
        col = 0;
        state_matrix[row][0] = 1;
    }

    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 1, 8);

    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 12);
    if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row))
    {
        col = 1;
        state_matrix[row][1] = 1;
    }
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 12);

    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 13);
    if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, 2, row))
    {
        col = 2;
        state_matrix[row][2] = 1;
    }

    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, 8);
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 12);
    return col;
}


void FILA0_IRQ_HANDLER(void)
{
    Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 4);
    NVIC_ClearPendingIRQ(PIN_INT4_IRQn);
    const uint8_t row = 0;
    const uint8_t col = read_row(row);
    last_char = row * 10 + col;
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH4);
}


void FILA1_IRQ_HANDLER(void)
{
    Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 5);
    NVIC_ClearPendingIRQ(PIN_INT5_IRQn);
    const uint8_t row = 1;
    const uint8_t col = read_row(row);
    last_char = row * 10 + col;
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH5);
}


void FILA2_IRQ_HANDLER(void)
{
    Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 6);
    NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
    const uint8_t row = 2;
    const uint8_t col = read_row(row);
    last_char = row * 10 + col;
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH6);
}


void FILA3_IRQ_HANDLER(void)
{
    Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT, 7);
    NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
    const uint8_t row = 3;
    const uint8_t col = read_row(row);
    last_char = row * 10 + col;
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH7);
}
