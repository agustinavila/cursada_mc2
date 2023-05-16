/**
 * @file uart_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "uart_driver.h"

LPC_USART_T* uart_channel_;

void driver_uart_init(uint8_t channel)
{
    switch (channel) {
    case 0:
        uart_channel_ = LPC_USART0;
        Chip_SCU_PinMux(9, 5, MD_PDN, FUNC7);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(9, 6, MD_PLN | MD_EZI | MD_ZI, FUNC7); /* P2 4: USART3 RXD */
        break;
    case 2:
        uart_channel_ = LPC_USART2;
        Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(7, 2, MD_PLN | MD_EZI | MD_ZI, FUNC6); /* P2 4: USART3 RXD */
        break;
    case 3:
        uart_channel_ = LPC_USART3;
        Chip_SCU_PinMux(2, 3, MD_PDN, FUNC2);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(2, 4, MD_PLN | MD_EZI | MD_ZI, FUNC2); /* P2 4: USART3 RXD */
        break;
    default:
        uart_channel_ = LPC_USART3;
        Chip_SCU_PinMux(2, 3, MD_PDN, FUNC2);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(2, 4, MD_PLN | MD_EZI | MD_ZI, FUNC2); /* P2 4: USART3 RXD */
        break;
    }
    Chip_UART_Init(uart_channel_);
    Chip_UART_SetBaud(uart_channel_, BAUD_115K);
    Chip_UART_SetupFIFOS(uart_channel_, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV0);
    Chip_UART_TXEnable(uart_channel_);
}

void driver_uart_int_enable(uint8_t rx_or_tx)
{
    NVIC_ClearPendingIRQ(USART3_IRQn);
    NVIC_EnableIRQ(USART3_IRQn);
    if (rx_or_tx == INT_RX) { Chip_UART_IntEnable(uart_channel_, UART_IER_RBRINT); }
    if (rx_or_tx == INT_TX) { Chip_UART_IntEnable(uart_channel_, UART_IER_THREINT); }
    if (rx_or_tx == (INT_TX | INT_RX)) { Chip_UART_IntEnable(uart_channel_, (UART_IER_RBRINT | UART_IER_THREINT)); }
}

void driver_uart_send_char(uint8_t data)
{ //
    Chip_UART_SendByte(uart_channel_, data);
}

void driver_uart_send_string(void* data, uint16_t numBytes)
{ //
    Chip_UART_SendBlocking(uart_channel_, data, numBytes);
}

void driver_uart_receive_char(void* data)
{ //
    Chip_UART_ReadBlocking(uart_channel_, data, 1);
}
