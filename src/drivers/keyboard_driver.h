/**
 * @file keyboard_driver.h
 * @brief Interfaz del driver de teclado matricial
 */

#if !defined(KEYBOARD_DRIVER_H_)
#define KEYBOARD_DRIVER_H_

#include "chip.h"
#include <stdint.h>

/** @brief Cantidad maxima de columnas del teclado matricial. */
#define KEYBOARD_MAX_COLUMNS 3
/** @brief Cantidad maxima de filas del teclado matricial. */
#define KEYBOARD_MAX_ROWS 4

/**
 * @brief Inicializa los pines del teclado matricial.
 */
void board_keyboard_init(void);

/**
 * @brief Habilita las interrupciones asociadas al teclado matricial.
 */
void board_keyboard_int_enable(void);

/**
 * @brief Lee el estado de una celda puntual de la matriz.
 *
 * @param column Indice de columna.
 * @param row Indice de fila.
 *
 * @return Estado leido para la celda indicada.
 */
int board_keyboard_read_cell(uint8_t column, uint8_t row);

/**
 * @brief Lee el estado completo de la matriz de teclado.
 *
 * @param matrix_p Vector de punteros donde se deposita el estado de cada columna.
 */
void board_keyboard_read_matrix(uint8_t* matrix_p[KEYBOARD_MAX_COLUMNS]);

/**
 * @brief Devuelve el ultimo caracter detectado por el teclado.
 *
 * @return Codigo del ultimo caracter leido.
 */
CHAR board_keyboard_get_last_char(void);

#endif // KEYBOARD_DRIVER_H_
