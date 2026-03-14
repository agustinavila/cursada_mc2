/**
 * @file uart_driver.h
 * @brief Interfaz del driver UART bloqueante
 */

#if !defined(DRIVER_UART_DRIVER_H)
#define DRIVER_UART_DRIVER_H

#include <chip.h>

/** @brief Habilita interrupcion de recepcion. */
#define INT_RX 1
/** @brief Habilita interrupcion de transmision. */
#define INT_TX 2

/** @brief Constante de baudrate a 9600 baudios. */
#define BAUD_9600 9600
/** @brief Constante de baudrate a 115200 baudios. */
#define BAUD_115K 115200

/**
 * @brief Inicializa un canal UART en modo bloqueante.
 *
 * @param channel Canal UART a utilizar.
 */
void driver_uart_init(uint8_t channel);

/**
 * @brief Habilita una interrupcion UART sobre el canal ya configurado.
 *
 * @param int_type Tipo de interrupcion a habilitar.
 */
void driver_uart_int_enable(uint8_t int_type);

/**
 * @brief Transmite un byte por UART en modo bloqueante.
 *
 * @param data Byte a transmitir.
 */
void driver_uart_send_char(uint8_t data);

/**
 * @brief Transmite una secuencia de bytes por UART en modo bloqueante.
 *
 * @param data Puntero al buffer a transmitir.
 * @param num_bytes Cantidad de bytes a transmitir.
 */
void driver_uart_send_string(const void* data, uint16_t num_bytes);

/**
 * @brief Recibe un byte por UART en modo bloqueante.
 *
 * @param data Puntero donde se almacenara el byte recibido.
 */
void driver_uart_receive_char(void* data);

#endif // DRIVER_UART_DRIVER_H
