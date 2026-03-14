/**
 * @file uart_driver_irq.h
 * @brief UART driver based on LPCOpen ring-buffer interrupt helpers
 */

#if !defined(DRIVER_UART_DRIVER_IRQ_H)
#define DRIVER_UART_DRIVER_IRQ_H

#include <chip.h>
#include <stdbool.h>
#include <stdint.h>

#define UART_IRQ_CHANNEL_0 0U
#define UART_IRQ_CHANNEL_2 2U
#define UART_IRQ_CHANNEL_3 3U

#define UART_IRQ_RX_BUFFER_SIZE 128U
#define UART_IRQ_TX_BUFFER_SIZE 128U

bool driver_uart_irq_init(uint8_t channel, uint32_t baudrate);

void driver_uart_irq_deinit(void);

int driver_uart_irq_write(const void* data, uint16_t num_bytes);

int driver_uart_irq_read(void* data, uint16_t num_bytes);

int driver_uart_irq_available(void);

int driver_uart_irq_tx_free(void);

void driver_uart_irq_flush(void);

bool driver_uart_irq_is_initialized(void);

void driver_uart_irq_handler(void);

#endif // DRIVER_UART_DRIVER_IRQ_H
