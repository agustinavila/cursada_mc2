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
#include <lpc_types.h>

void driver_lcd_init(void);

void driver_lcd_set_position(uint8_t x, uint8_t y);

void driver_lcd_write_char(char c);

void driver_lcd_printf(char* string);


#endif // DRIVER_LCD_DRIVER_H_
