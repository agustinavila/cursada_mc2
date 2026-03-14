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
#include "control/control.h"
#include "control/control_selector.h"
#include "control/control_on_off.h"
#include "hmi/hmi.h"

static control_selector_t app_selector_control_;
static const uint8_t app_indice_sensor_proceso_ = 0U;

static const control_on_off_configuracion_t app_control_on_off_configuracion_inicial_ = {
    .sentido = CONTROL_ON_OFF_SENTIDO_CALENTAR,
    .consigna_deci_celsius = 270,
    .histeresis_deci_celsius = 20U,
    .habilitado = true,
};

static void app_actualizar_control(void)
{
    control_generico_t* control_activo = 0;
    int16_t temperatura_deci_celsius = 0;

    control_activo = control_selector_obtener_activo(&app_selector_control_);

    if ((control_activo == 0)
        || !hmi_obtener_temperatura_sensor(app_indice_sensor_proceso_, &temperatura_deci_celsius)) {
        led_turn_off(LED1);
        return;
    }

    if (!control_procesar(control_activo, temperatura_deci_celsius)) {
        led_turn_off(LED1);
        return;
    }

    if (control_esta_salida_activa(control_activo)) {
        led_turn_on(LED1);
    } else {
        led_turn_off(LED1);
    }
}

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
    (void) control_selector_inicializar(&app_selector_control_);
    (void) control_selector_seleccionar_on_off(&app_selector_control_,
                                               &app_control_on_off_configuracion_inicial_);
}

void app_process(void)
{
    hmi_process();
    app_actualizar_control();
}
