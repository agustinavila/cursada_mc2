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


typedef struct __LCD_DATA_Type
{
    Bool D4;
    Bool D3;
    Bool D2;
    Bool D1;
    Bool RS;
    Bool EN;
} LCD_DATA_Type;

#define LCD_PORT 4 // Donde esta el LCD
#define LCD4 10    // Pines del puerto
#define LCD3 6
#define LCD2 5
#define LCD1 4
#define LCD_RS 8
#define LCD_EN 9

void lcd_init_port(void);
void lcd_init(void);
void lcd_gotoxy(int x, int y);
void lcd_putc(char c);
// void enviar_lcd(LCD_DATA_Type data);
void printf_lcd(char* string);
void delay(void);


#endif // DRIVER_LCD_DRIVER_H_
