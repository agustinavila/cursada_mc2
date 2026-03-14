/**
 * @file app.c
 * @brief Implementacion de la capa principal de aplicacion.
 */

#include "app/app.h"

#include "Driver/adc_driver.h"
#include "Driver/buttons_driver.h"
#include "Driver/buzzer_driver.h"
#include "Driver/keyboard_driver.h"
#include "Driver/lcd_driver.h"
#include "Driver/led_Driver.h"
#include "hmi/hmi.h"

void app_init(void)
{
    led_init();
    buzzer_init();
    buzzer_turn_off();
    buttons_init();
    board_keyboard_init();
    board_adc_init(ADC_CH2);
    driver_lcd_init();
    hmi_init();
}

void app_process(void)
{
    hmi_process();
}
