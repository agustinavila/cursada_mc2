/**
 * @file app.c
 * @brief Implementacion de la capa principal de aplicacion.
 */

#include "app/app.h"

#include "drivers/buttons_driver.h"
#include "drivers/buzzer_driver.h"
#include "drivers/delay_driver.h"
#include "drivers/ds18b20_driver.h"
#include "drivers/eeprom_driver.h"
#include "drivers/lcd_driver.h"
#include "drivers/led_driver.h"
#include "drivers/timer_driver.h"
#include "app/parametros.h"
#include "control/control_on_off.h"
#include "hmi/hmi.h"

static control_on_off_configuracion_t app_control_on_off_configuracion_actual_;

typedef struct {
    ds18b20_driver_t sensor_temperatura;
    bool inicializado;
    uint16_t ticks_actualizacion;
} app_sensores_t;

static app_sensores_t app_sensores_;

static const onewire_pin_config_t app_pin_ds18b20_ = {
    .scu_port = 6U,
    .scu_pin = 1U,
    .scu_mode = (uint16_t) (MD_PUP | MD_EZI | MD_ZI),
    .scu_func = FUNC0,
    .gpio_port = 3U,
    .gpio_pin = 0U,
};

#define APP_LOOP_DELTA_MS 20U
#define APP_TIMER_TICK_MS 1U

static uint32_t app_ultimo_tick_procesado_ms_ = 0U;

static int16_t app_convertir_temperatura_raw_a_deci(int16_t temperatura_cruda)
{
    const int32_t temperatura_escalada = (int32_t) temperatura_cruda * 10;

    if (temperatura_escalada >= 0) {
        return (int16_t) ((temperatura_escalada + 8) / 16);
    }

    return (int16_t) ((temperatura_escalada - 8) / 16);
}

static bool app_obtener_temperatura_sensor_principal(int16_t* temperatura_deci_celsius)
{
    int16_t temperatura_cruda = 0;

    if ((temperatura_deci_celsius == 0) || !app_sensores_.inicializado) {
        return false;
    }

    if (!ds18b20_get_latest_raw(&app_sensores_.sensor_temperatura, &temperatura_cruda)) {
        return false;
    }

    *temperatura_deci_celsius = app_convertir_temperatura_raw_a_deci(temperatura_cruda);
    return true;
}

static void app_cargar_sensor_en_hmi(void)
{
    int16_t temperatura_deci_celsius = 0;

    if (app_obtener_temperatura_sensor_principal(&temperatura_deci_celsius)) {
        hmi_cargar_estado_sensor(true, temperatura_deci_celsius);
    } else {
        hmi_cargar_estado_sensor(false, 0);
    }
}

static void app_actualizar_sensores(void)
{
    if (!app_sensores_.inicializado) {
        app_cargar_sensor_en_hmi();
        return;
    }

    ds18b20_process(&app_sensores_.sensor_temperatura, 20U);

    if (!ds18b20_is_busy(&app_sensores_.sensor_temperatura)) {
        app_sensores_.ticks_actualizacion++;
        if (app_sensores_.ticks_actualizacion >= 50U) {
            app_sensores_.ticks_actualizacion = 0U;
            (void) ds18b20_start_conversion(&app_sensores_.sensor_temperatura);
        }
    } else {
        app_sensores_.ticks_actualizacion = 0U;
    }

    app_cargar_sensor_en_hmi();
}

static void app_cargar_parametros_en_hmi(void)
{
    const parametros_t* parametros = parametros_obtener();

    hmi_cargar_parametros_control(parametros->control.setpoint_deci_celsius,
                                  parametros->control.histeresis_deci_celsius,
                                  parametros->control.tiempo_minimo_encendido_ms,
                                  parametros->control.tiempo_minimo_apagado_ms,
                                  parametros->control.modo_calentar);
}

static control_on_off_configuracion_t app_obtener_configuracion_control_desde_parametros(void)
{
    const parametros_t* parametros = parametros_obtener();
    control_on_off_configuracion_t configuracion = {
        .sentido = parametros->control.modo_calentar
            ? CONTROL_ON_OFF_SENTIDO_CALENTAR
            : CONTROL_ON_OFF_SENTIDO_ENFRIAR,
        .setpoint_deci_celsius = parametros->control.setpoint_deci_celsius,
        .histeresis_deci_celsius = parametros->control.histeresis_deci_celsius,
        .tiempo_minimo_encendido_ms = parametros->control.tiempo_minimo_encendido_ms,
        .tiempo_minimo_apagado_ms = parametros->control.tiempo_minimo_apagado_ms,
    };

    return configuracion;
}

static void app_sincronizar_control_desde_parametros(void)
{
    control_on_off_configuracion_t nueva_configuracion = app_obtener_configuracion_control_desde_parametros();

    if ((nueva_configuracion.setpoint_deci_celsius == app_control_on_off_configuracion_actual_.setpoint_deci_celsius)
        && (nueva_configuracion.histeresis_deci_celsius == app_control_on_off_configuracion_actual_.histeresis_deci_celsius)
        && (nueva_configuracion.sentido == app_control_on_off_configuracion_actual_.sentido)
        && (nueva_configuracion.tiempo_minimo_encendido_ms
            == app_control_on_off_configuracion_actual_.tiempo_minimo_encendido_ms)
        && (nueva_configuracion.tiempo_minimo_apagado_ms
            == app_control_on_off_configuracion_actual_.tiempo_minimo_apagado_ms)) {
        return;
    }

    app_control_on_off_configuracion_actual_ = nueva_configuracion;
    control_on_off_configurar(app_control_on_off_configuracion_actual_);
}

static void app_sincronizar_hmi_en_parametros(void)
{
    const hmi_parametros_control_t parametros_hmi = hmi_obtener_parametros_control();

    if (parametros_actualizar_control(parametros_hmi.setpoint_deci_celsius,
                                      parametros_hmi.histeresis_deci_celsius,
                                      parametros_hmi.tiempo_minimo_encendido_ms,
                                      parametros_hmi.tiempo_minimo_apagado_ms,
                                      parametros_hmi.modo_calentar)) {
        (void) parametros_guardar();
    }
}

static void app_actualizar_control(void)
{
    int16_t temperatura_deci_celsius = 0;
    bool salida_activa = false;

    app_sincronizar_hmi_en_parametros();
    app_sincronizar_control_desde_parametros();

    if (!app_obtener_temperatura_sensor_principal(&temperatura_deci_celsius)) {
        hmi_cargar_estado_control(false, false);
        led_turn_off(LED1);
        return;
    }

    control_on_off_procesar(temperatura_deci_celsius, APP_LOOP_DELTA_MS);
    salida_activa = control_on_off_esta_salida_activa();
    hmi_cargar_estado_control(salida_activa, true);

    if (salida_activa) {
        led_turn_on(LED1);
    } else {
        led_turn_off(LED1);
    }
}

void app_init(void)
{
    driver_delay_init();
    board_timer_init(APP_TIMER_TICK_MS);
    led_init();
    buzzer_init();
    buzzer_turn_off();
    buttons_init();
    driver_lcd_init();
    (void) driver_eeprom_init();
    (void) parametros_init();

    app_sensores_.inicializado = ds18b20_init(&app_sensores_.sensor_temperatura, &app_pin_ds18b20_);
    if (app_sensores_.inicializado) {
        (void) ds18b20_start_conversion(&app_sensores_.sensor_temperatura);
    }

    app_cargar_sensor_en_hmi();
    hmi_init();
    app_cargar_parametros_en_hmi();

    app_control_on_off_configuracion_actual_ = app_obtener_configuracion_control_desde_parametros();
    control_on_off_inicializar(app_control_on_off_configuracion_actual_);
    app_ultimo_tick_procesado_ms_ = board_timer_get_ticks();
}

void app_process(void)
{
    const uint32_t tick_actual_ms = board_timer_get_ticks();

    while ((uint32_t) (tick_actual_ms - app_ultimo_tick_procesado_ms_) >= APP_LOOP_DELTA_MS) {
        app_ultimo_tick_procesado_ms_ += APP_LOOP_DELTA_MS;

        app_actualizar_sensores();
        buttons_process(APP_LOOP_DELTA_MS);
        hmi_process();
        app_actualizar_control();
    }
}
