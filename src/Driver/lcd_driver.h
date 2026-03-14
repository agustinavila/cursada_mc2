/**
 * @file lcd_driver.h
 * @brief Interfaz del driver para LCD alfanumerico
 */

#if !defined(DRIVER_LCD_DRIVER_H_)
#define DRIVER_LCD_DRIVER_H_

#include <chip.h>

/**
 * @brief Inicializa el LCD y ejecuta la secuencia de arranque.
 */
void driver_lcd_init(void);

/**
 * @brief Posiciona el cursor dentro de la matriz visible del LCD.
 *
 * @param x Columna, en el rango 1 a 16.
 * @param y Fila, en el rango 1 a 2.
 */
void driver_lcd_set_position(uint8_t x, uint8_t y);

/**
 * @brief Escribe un caracter en la posicion actual del LCD.
 *
 * @param c Caracter a escribir o caracter de control soportado por el driver.
 */
void driver_lcd_write_char(char c);

/**
 * @brief Escribe una cadena a partir de la posicion actual del cursor.
 *
 * @param string Cadena a imprimir.
 */
void driver_lcd_printf(const char* string);

#endif // DRIVER_LCD_DRIVER_H_
