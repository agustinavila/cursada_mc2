/**
 * @file lcd_driver.h
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(DRIVER_LCD_DRIVER_H_)
#define DRIVER_LCD_DRIVER_H_

#include <chip.h>

/// @brief Initializes LCD pins, sends starting commands to the LCD MCU
void driver_lcd_init();

/**
 * @brief Sets the cursor position
 * 
 * @param x X position (column) - 1 to 16
 * @param y Y position (row) - 1 or 2
 */
void driver_lcd_set_position(uint8_t x, uint8_t y);

/**
 * @brief writes character
 * @param C Character to write
 */
void driver_lcd_write_char(char c);

/**
 * @brief prints a string starting from the current position
 * 
 * @param string String to print
 * @todo it doesn't check matrix boundaries
 */
void driver_lcd_printf(char* string);


#endif // DRIVER_LCD_DRIVER_H_
