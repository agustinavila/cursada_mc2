#ifndef TECLAS_PIN_INT
#define TECLAS_PIN_INT

#include "chip.h"

#define PININT0_IRQ_HANDLER GPIO0_IRQHandler
#define PININT1_IRQ_HANDLER GPIO1_IRQHandler
#define PININT2_IRQ_HANDLER GPIO2_IRQHandler
#define PININT3_IRQ_HANDLER GPIO3_IRQHandler



void teclas_Init_Int(uint8_t);

#endif // TECLAS_PIN_INT
