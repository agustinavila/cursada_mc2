/**
 * @file uart_driver_irq.h
 * @brief Driver UART por interrupciones basado en ring buffers de LPCOpen
 */

#if !defined(DRIVER_UART_DRIVER_IRQ_H)
#define DRIVER_UART_DRIVER_IRQ_H

#include <chip.h>
#include <stdbool.h>
#include <stdint.h>

/** @brief Identificador del canal UART0. */
#define UART_IRQ_CHANNEL_0 0U
/** @brief Identificador del canal UART2. */
#define UART_IRQ_CHANNEL_2 2U
/** @brief Identificador del canal UART3. */
#define UART_IRQ_CHANNEL_3 3U

/** @brief Tamano del buffer circular de recepcion. */
#define UART_IRQ_RX_BUFFER_SIZE 128U
/** @brief Tamano del buffer circular de transmision. */
#define UART_IRQ_TX_BUFFER_SIZE 128U

/**
 * @brief Inicializa un canal UART para operar por interrupciones.
 *
 * Configura el pinmux, la UART seleccionada, los buffers circulares y la IRQ
 * correspondiente al canal.
 *
 * @param channel Canal UART a utilizar.
 * @param baudrate Velocidad de comunicacion en baudios.
 *
 * @retval true Si la inicializacion fue correcta.
 * @retval false Si el canal solicitado no es valido.
 */
bool driver_uart_irq_init(uint8_t channel, uint32_t baudrate);

/**
 * @brief Desinicializa la UART configurada y deshabilita sus interrupciones.
 */
void driver_uart_irq_deinit(void);

/**
 * @brief Encola datos para transmision por UART.
 *
 * @param data Puntero al buffer de datos a transmitir.
 * @param num_bytes Cantidad de bytes a transmitir.
 *
 * @return Cantidad de bytes efectivamente encolados.
 */
int driver_uart_irq_write(const void* data, uint16_t num_bytes);

/**
 * @brief Lee datos disponibles desde el buffer de recepcion.
 *
 * @param data Puntero al buffer destino.
 * @param num_bytes Cantidad maxima de bytes a leer.
 *
 * @return Cantidad de bytes efectivamente leidos.
 */
int driver_uart_irq_read(void* data, uint16_t num_bytes);

/**
 * @brief Informa cuantos bytes hay disponibles para leer.
 *
 * @return Cantidad de bytes pendientes en el buffer RX.
 */
int driver_uart_irq_available(void);

/**
 * @brief Informa cuanto espacio libre queda en el buffer de transmision.
 *
 * @return Cantidad de bytes libres en el buffer TX.
 */
int driver_uart_irq_tx_free(void);

/**
 * @brief Vacia los buffers de recepcion y transmision.
 */
void driver_uart_irq_flush(void);

/**
 * @brief Informa si el driver ya fue inicializado.
 *
 * @retval true Si el driver se encuentra operativo.
 * @retval false Si el driver aun no fue inicializado.
 */
bool driver_uart_irq_is_initialized(void);

/**
 * @brief Handler generico del driver para ser invocado desde la IRQ activa.
 *
 * Normalmente no se llama desde la aplicacion. Se expone para permitir
 * redireccionar handlers si fuera necesario.
 */
void driver_uart_irq_handler(void);

#endif // DRIVER_UART_DRIVER_IRQ_H
