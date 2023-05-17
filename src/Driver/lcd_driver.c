/**
 * @file lcd_driver.c
 * @author agustinavila (tinto.avila@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "lcd_driver.h"

#define LCD_PORT 4
#define LCD4 10 ///< LCD4 = D7 on lcd pinout
#define LCD3 6  ///< LCD3 = D6 on lcd pinout
#define LCD2 5  ///< LCD2 = D5 on lcd pinout
#define LCD1 4  ///< LCD1 = D4 on lcd pinout
#define LCD_RS 8
#define LCD_EN 9

#define LCD_IS_DATA 1
#define LCD_IS_COMMAND 0


void delay(void)
{
    uint16_t x = 0;
    for (uint16_t i = 0; i < 9999; i++) { x++; }
}


void set_value(uint8_t port, uint8_t pin, bool data)
{
    if (data) { Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, port, pin); }
    else {
        Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, port, pin);
    }
}


void lcd_send(uint8_t nibble, bool is_data)
{
    const bool bit_0 = (bool) (nibble & 0x01);
    set_value(2, LCD1, bit_0);

    const bool bit_1 = (bool) ((nibble >> 1) & 0x01);
    set_value(2, LCD2, bit_1);

    const bool bit_2 = (bool) ((nibble >> 2) & 0x01);
    set_value(2, LCD3, bit_2);

    const bool bit_3 = (bool) ((nibble >> 3) & 0x01);
    set_value(5, LCD4 + 4, bit_3);

    set_value(5, LCD_RS + 4, is_data);
    // delay();
    // Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, LCD_EN + 4);
    delay();
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, LCD_EN + 4);
    delay();
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, LCD_EN + 4);
    delay();
}


void send_command(uint8_t command)
{
    const uint8_t upper_nibble = (command >> 4) & 0x0F;
    lcd_send(upper_nibble, LCD_IS_COMMAND);
    delay();
    const uint8_t lower_nibble = command & 0x0F;
    lcd_send(lower_nibble, LCD_IS_COMMAND);
    delay();
}


void send_data(uint8_t data)
{
    const uint8_t upper_nibble = (data >> 4) & 0x0F;
    lcd_send(upper_nibble, LCD_IS_DATA);
    delay();
    const uint8_t lower_nibble = data & 0x0F;
    lcd_send(lower_nibble, LCD_IS_DATA);
    delay();
}


void driver_lcd_init_port(void)
{
    Chip_SCU_PinMux(LCD_PORT, LCD_RS, MD_PLN, FUNC4);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 5, LCD_RS + 4, 1);

    Chip_SCU_PinMux(LCD_PORT, LCD_EN, MD_PLN, FUNC4);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 5, LCD_EN + 4, 1);

    Chip_SCU_PinMux(LCD_PORT, LCD4, MD_PLN, FUNC4);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 5, LCD4 + 4, 1);

    Chip_SCU_PinMux(LCD_PORT, LCD3, MD_PLN, FUNC0);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 2, LCD3, 1);

    Chip_SCU_PinMux(LCD_PORT, LCD2, MD_PLN, FUNC0);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 2, LCD2, 1);

    Chip_SCU_PinMux(LCD_PORT, LCD1, MD_PLN, FUNC0);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, 2, LCD1, 1);
}


void driver_lcd_init(void)
{
    delay();
    // Function set (interface is 8 bits long)
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    delay();
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    delay();
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    delay();
    lcd_send(0x02, LCD_IS_COMMAND);
    delay();
    delay();

    // Function set: interface is 4-bit long, 5x8 dot font
    // DL = 1; 8-bit interface data
    // N = 0; 1-line display
    // F = 0; 5 × 8 dot character font
    send_command(0x2F); // 0b0010 ' N F x x
    send_command(0x08); // Display off
    send_command(0x01); // Display clear

    // Entry mode set
    // I/D = 1; Increment by 1
    // S = 0; No shift
    send_command(0x06); // 0b0000 ' 0 1 I/D S

    send_command(0x0C); // Display on, cursor off
}


void driver_lcd_set_position(uint8_t x, uint8_t y)
{
    if (x < 1 || x > 16) return;
    if (y < 1 || y > 2) return; // checks limits of display (16x2)

    uint8_t address_position = (y == 1) ? 0x00 : 0x40;
    address_position += (uint8_t) (x - 1);
    address_position |= 0x80; // this bit must be 1
    send_command(address_position);
}


void driver_lcd_write_char(char C)
{
    switch (C) {
    case '\f':
        break;
    case '\n':
        break;
    case '\b':
        send_command(0x01); // Clear display screen
        break;
    default:
        send_data(C);
        break;
    }
}


void driver_lcd_printf(char* string)
{
    uint16_t i = 0;
    while (string[i] != '\0') {
        driver_lcd_write_char(string[i]);
        i++;
    }
}
