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

void Board_Keyboard_Init(void);

void Board_Keyboard_IntEnable(void);

void Board_Keyboard_tick_ms(void);

int Board_Keyboard_readCell(uint8_t, uint8_t);

void Board_Keyboard_readMatrix(uint8_t *matrix_p);

