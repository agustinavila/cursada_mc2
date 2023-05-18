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


void lcd_send(uint8_t nibble, bool is_data)
{
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, LCD_RS + 4, is_data);

    const bool bit_0 = (bool) (nibble & 0x01);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2, LCD1, bit_0);

    const bool bit_1 = (bool) ((nibble >> 1) & 0x01);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2, LCD2, bit_1);

    const bool bit_2 = (bool) ((nibble >> 2) & 0x01);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2, LCD3, bit_2);

    const bool bit_3 = (bool) ((nibble >> 3) & 0x01);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, LCD4 + 4, bit_3);

    delay();
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, LCD_EN + 4);
    delay();
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, LCD_EN + 4);
    delay();
}


void send_byte(uint8_t payload, bool data_type)
{
    const uint8_t upper_nibble = (payload >> 4) & 0x0F;
    lcd_send(upper_nibble, data_type);
    delay();
    const uint8_t lower_nibble = payload & 0x0F;
    lcd_send(lower_nibble, data_type);
    delay();
}


void driver_lcd_init_port()
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
    driver_lcd_init_port();
    delay();
    // Function set (interface is 8 bits long)
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    delay();
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    lcd_send(0x03, LCD_IS_COMMAND);
    delay();
    lcd_send(0x02, LCD_IS_COMMAND);
    delay();


    // Function set: interface is 4-bit long, 5x8 dot font
    // DL = 1; 8-bit interface data
    // N = 0; 1-line display
    // F = 0; 5 × 8 dot character font
    send_byte(0x2F, LCD_IS_COMMAND); // 0b0010 ' N F x x
    send_byte(0x08, LCD_IS_COMMAND); // Display off
    send_byte(0x01, LCD_IS_COMMAND); // Display clear

    // Entry mode set
    // I/D = 1; Increment by 1
    // S = 0; No shift
    send_byte(0x06, LCD_IS_COMMAND); // 0b0000 ' 0 1 I/D S
    send_byte(0x0C, LCD_IS_COMMAND); // Display on, cursor off
}


void driver_lcd_set_position(uint8_t x, uint8_t y)
{
    if (x < 1 || x > 16) return;
    if (y < 1 || y > 2) return; // checks limits of display (16x2)

    uint8_t address_position = (y == 1) ? 0x80 : 0xC0;
    address_position += (uint8_t) (x - 1);
    send_byte(address_position, LCD_IS_COMMAND);
}


void driver_lcd_write_char(char C)
{
    switch (C) {
    case '\f':
        break;
    case '\n':
        break;
    case '\b':
        send_byte(0x01, LCD_IS_COMMAND); // Clear display screen
        break;
    default:
        send_byte(C, LCD_IS_DATA);
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
