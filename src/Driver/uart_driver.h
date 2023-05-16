/**
 * @file uart_driver.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(DRIVER_UART_DRIVER_H)
#define DRIVER_UART_DRIVER_H

#include <chip.h>

#define INT_RX 1
#define INT_TX 2

#define BAUD_9600 9600
#define BAUD_115K 115200

void driver_uart_init(uint8_t);

void driver_uart_int_enable(uint8_t);

void driver_uart_send_char(uint8_t);

void driver_uart_send_string(void*, uint16_t);

void driver_uart_receive_char(void* data);


#endif // DRIVER_UART_DRIVER_H
