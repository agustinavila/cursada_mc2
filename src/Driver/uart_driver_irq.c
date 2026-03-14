/**
 * @file uart_driver_irq.c
 * @brief UART driver based on LPCOpen ring-buffer interrupt helpers
 */

#include "uart_driver_irq.h"

#include <ring_buffer.h>

typedef struct {
    LPC_USART_T* peripheral;
    IRQn_Type irq;
    uint8_t channel;
    bool initialized;
    RINGBUFF_T rx_ring;
    RINGBUFF_T tx_ring;
    uint8_t rx_data[UART_IRQ_RX_BUFFER_SIZE];
    uint8_t tx_data[UART_IRQ_TX_BUFFER_SIZE];
} driver_uart_irq_state_t;

static driver_uart_irq_state_t driver_uart_irq_state_;

static bool driver_uart_irq_select_channel(uint8_t channel)
{
    switch (channel) {
    case UART_IRQ_CHANNEL_0:
        driver_uart_irq_state_.peripheral = LPC_USART0;
        driver_uart_irq_state_.irq = USART0_IRQn;
        driver_uart_irq_state_.channel = channel;
        Chip_SCU_PinMux(9, 5, MD_PDN, FUNC7);
        Chip_SCU_PinMux(9, 6, MD_PLN | MD_EZI | MD_ZI, FUNC7);
        return true;
    case UART_IRQ_CHANNEL_2:
        driver_uart_irq_state_.peripheral = LPC_USART2;
        driver_uart_irq_state_.irq = USART2_IRQn;
        driver_uart_irq_state_.channel = channel;
        Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);
        Chip_SCU_PinMux(7, 2, MD_PLN | MD_EZI | MD_ZI, FUNC6);
        return true;
    case UART_IRQ_CHANNEL_3:
        driver_uart_irq_state_.peripheral = LPC_USART3;
        driver_uart_irq_state_.irq = USART3_IRQn;
        driver_uart_irq_state_.channel = channel;
        Chip_SCU_PinMux(2, 3, MD_PDN, FUNC2);
        Chip_SCU_PinMux(2, 4, MD_PLN | MD_EZI | MD_ZI, FUNC2);
        return true;
    default:
        return false;
    }
}

bool driver_uart_irq_init(uint8_t channel, uint32_t baudrate)
{
    if (!driver_uart_irq_select_channel(channel)) {
        return false;
    }

    RingBuffer_Init(&driver_uart_irq_state_.rx_ring,
                    driver_uart_irq_state_.rx_data,
                    sizeof(driver_uart_irq_state_.rx_data[0]),
                    UART_IRQ_RX_BUFFER_SIZE);
    RingBuffer_Init(&driver_uart_irq_state_.tx_ring,
                    driver_uart_irq_state_.tx_data,
                    sizeof(driver_uart_irq_state_.tx_data[0]),
                    UART_IRQ_TX_BUFFER_SIZE);

    Chip_UART_Init(driver_uart_irq_state_.peripheral);
    Chip_UART_SetBaud(driver_uart_irq_state_.peripheral, baudrate);
    Chip_UART_SetupFIFOS(driver_uart_irq_state_.peripheral,
                         UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV0);
    Chip_UART_TXEnable(driver_uart_irq_state_.peripheral);

    Chip_UART_IntDisable(driver_uart_irq_state_.peripheral,
                         UART_IER_RBRINT | UART_IER_THREINT | UART_IER_RLSINT);
    Chip_UART_IntEnable(driver_uart_irq_state_.peripheral, UART_IER_RBRINT | UART_IER_RLSINT);

    NVIC_ClearPendingIRQ(driver_uart_irq_state_.irq);
    NVIC_EnableIRQ(driver_uart_irq_state_.irq);

    driver_uart_irq_state_.initialized = true;
    return true;
}

void driver_uart_irq_deinit(void)
{
    if (!driver_uart_irq_state_.initialized) {
        return;
    }

    NVIC_DisableIRQ(driver_uart_irq_state_.irq);
    Chip_UART_IntDisable(driver_uart_irq_state_.peripheral,
                         UART_IER_RBRINT | UART_IER_THREINT | UART_IER_RLSINT);
    Chip_UART_DeInit(driver_uart_irq_state_.peripheral);
    driver_uart_irq_state_.initialized = false;
}

int driver_uart_irq_write(const void* data, uint16_t num_bytes)
{
    if (!driver_uart_irq_state_.initialized || (data == 0) || (num_bytes == 0U)) {
        return 0;
    }

    return (int) Chip_UART_SendRB(driver_uart_irq_state_.peripheral,
                                  &driver_uart_irq_state_.tx_ring,
                                  data,
                                  (int) num_bytes);
}

int driver_uart_irq_read(void* data, uint16_t num_bytes)
{
    if (!driver_uart_irq_state_.initialized || (data == 0) || (num_bytes == 0U)) {
        return 0;
    }

    return Chip_UART_ReadRB(driver_uart_irq_state_.peripheral,
                            &driver_uart_irq_state_.rx_ring,
                            data,
                            (int) num_bytes);
}

int driver_uart_irq_available(void)
{
    if (!driver_uart_irq_state_.initialized) {
        return 0;
    }

    return RingBuffer_GetCount(&driver_uart_irq_state_.rx_ring);
}

int driver_uart_irq_tx_free(void)
{
    if (!driver_uart_irq_state_.initialized) {
        return 0;
    }

    return RingBuffer_GetFree(&driver_uart_irq_state_.tx_ring);
}

void driver_uart_irq_flush(void)
{
    if (!driver_uart_irq_state_.initialized) {
        return;
    }

    RingBuffer_Flush(&driver_uart_irq_state_.rx_ring);
    RingBuffer_Flush(&driver_uart_irq_state_.tx_ring);
}

bool driver_uart_irq_is_initialized(void)
{
    return driver_uart_irq_state_.initialized;
}

void driver_uart_irq_handler(void)
{
    if (!driver_uart_irq_state_.initialized) {
        return;
    }

    Chip_UART_IRQRBHandler(driver_uart_irq_state_.peripheral,
                           &driver_uart_irq_state_.rx_ring,
                           &driver_uart_irq_state_.tx_ring);
}

void UART0_IRQHandler(void)
{
    if (driver_uart_irq_state_.channel == UART_IRQ_CHANNEL_0) {
        driver_uart_irq_handler();
    }
}

void UART2_IRQHandler(void)
{
    if (driver_uart_irq_state_.channel == UART_IRQ_CHANNEL_2) {
        driver_uart_irq_handler();
    }
}

void UART3_IRQHandler(void)
{
    if (driver_uart_irq_state_.channel == UART_IRQ_CHANNEL_3) {
        driver_uart_irq_handler();
    }
}
