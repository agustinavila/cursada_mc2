/**
 * @file teclado.h
 * @author Agustin Avila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "stdint.h"
#include "chip.h"

#define KEYBOARD_MAX_COLUMNS 3
#define KEYBOARD_MAX_ROWS 4

void board_keyboard_init(void);

void board_Keyboard_int_enable(void);

void Board_Keyboard_tick_ms(void);

int Board_Keyboard_readCell(uint8_t, uint8_t);

void board_keyboard_read_matrix(uint8_t *matrix_p);

