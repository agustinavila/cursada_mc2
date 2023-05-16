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


LPC_USART_T* channel;

void driver_uart_init(uint8_t canal)
{
    switch (canal) {
    case 0:
        channel = LPC_USART0;
        Chip_SCU_PinMux(9, 5, MD_PDN, FUNC7);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(9, 6, MD_PLN | MD_EZI | MD_ZI, FUNC7); /* P2 4: USART3 RXD */
        break;
    case 2:
        channel = LPC_USART2;
        Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(7, 2, MD_PLN | MD_EZI | MD_ZI, FUNC6); /* P2 4: USART3 RXD */
        break;
    case 3:
        channel = LPC_USART3;
        Chip_SCU_PinMux(2, 3, MD_PDN, FUNC2);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(2, 4, MD_PLN | MD_EZI | MD_ZI, FUNC2); /* P2 4: USART3 RXD */
        break;
    default:
        channel = LPC_USART3;
        Chip_SCU_PinMux(2, 3, MD_PDN, FUNC2);                  /* P2 3: USART3 TXD */
        Chip_SCU_PinMux(2, 4, MD_PLN | MD_EZI | MD_ZI, FUNC2); /* P2 4: USART3 RXD */
        break;
    }
    Chip_UART_Init(channel);
    Chip_UART_SetBaud(channel, BAUD_115K);
    Chip_UART_SetupFIFOS(channel, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV0);
    Chip_UART_TXEnable(channel);
}

void driver_uart_int_enable(uint8_t RxTx)
{
    NVIC_ClearPendingIRQ(USART3_IRQn);
    NVIC_EnableIRQ(USART3_IRQn);
    if (RxTx == INT_RX) { Chip_UART_IntEnable(channel, UART_IER_RBRINT); }
    if (RxTx == INT_TX) { Chip_UART_IntEnable(channel, UART_IER_THREINT); }
    if (RxTx == (INT_TX | INT_RX)) { Chip_UART_IntEnable(channel, (UART_IER_RBRINT | UART_IER_THREINT)); }
}

void driver_uart_send_char(uint8_t data) { Chip_UART_SendByte(channel, data); }

void driver_uart_send_string(void* data, uint16_t numBytes) { Chip_UART_SendBlocking(channel, data, numBytes); }

void driver_uart_receive_char(void* data) { Chip_UART_ReadBlocking(channel, data, 1); }
