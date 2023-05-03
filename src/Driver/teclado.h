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

#include "chip.h"
#include "stdint.h"


#define KEYBOARD_MAX_COLUMNS 3
#define KEYBOARD_MAX_ROWS 4

void board_keyboard_init(void);

void board_keyboard_int_enable(void);

// void board_keyboard_tick_ms(void); // TODO enable this

int board_keyboard_read_cell(uint8_t, uint8_t);

void board_keyboard_read_matrix(uint8_t* matrix_p[KEYBOARD_MAX_COLUMNS]);

CHAR board_keyboard_get_last_char();
