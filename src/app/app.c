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

static void app_step_20ms(void)
{
    const hmi_parametros_control_t parametros_hmi = hmi_obtener_parametros_control();
    hmi_estado_proceso_t estado_hmi = {0};
    const parametros_control_t* parametros = 0;
    const parametros_control_t parametros_hmi_convertidos = {
        .setpoint_deci_celsius = parametros_hmi.setpoint_deci_celsius,
        .histeresis_deci_celsius = parametros_hmi.histeresis_deci_celsius,
        .tiempo_minimo_encendido_ms = parametros_hmi.tiempo_minimo_encendido_ms,
        .tiempo_minimo_apagado_ms = parametros_hmi.tiempo_minimo_apagado_ms,
        .modo_calentar = parametros_hmi.modo_calentar,
    };
    int16_t temperatura_cruda = 0;
    int16_t temperatura_deci_celsius = 0;
    bool temperatura_valida = false;
    bool salida_activa = false;

    if (app_sensores_.inicializado) {
        ds18b20_process(&app_sensores_.sensor_temperatura, APP_LOOP_DELTA_MS);

        if (!ds18b20_is_busy(&app_sensores_.sensor_temperatura)) {
            app_sensores_.ticks_actualizacion++;
            if (app_sensores_.ticks_actualizacion >= 50U) {
                app_sensores_.ticks_actualizacion = 0U;
                (void) ds18b20_start_conversion(&app_sensores_.sensor_temperatura);
            }
        } else {
            app_sensores_.ticks_actualizacion = 0U;
        }

        // Lee la ultima conversion lista del DS18B20 y la pasa a decimas de grado.
        if (ds18b20_get_latest_raw(&app_sensores_.sensor_temperatura, &temperatura_cruda)) {
            temperatura_deci_celsius = app_convertir_temperatura_raw_a_deci(temperatura_cruda);
            temperatura_valida = true;
        }
    }

    estado_hmi.temperatura_valida = temperatura_valida;
    estado_hmi.temperatura_deci_celsius = temperatura_valida ? temperatura_deci_celsius : 0;

    buttons_process(APP_LOOP_DELTA_MS);
    hmi_process();

    if (parametros_actualizar(&parametros_hmi_convertidos)) {
        (void) parametros_guardar();
    }

    parametros = parametros_obtener();
    control_on_off_configurar((control_on_off_configuracion_t) {
        .sentido = parametros->modo_calentar
            ? CONTROL_ON_OFF_SENTIDO_CALENTAR
            : CONTROL_ON_OFF_SENTIDO_ENFRIAR,
        .setpoint_deci_celsius = parametros->setpoint_deci_celsius,
        .histeresis_deci_celsius = parametros->histeresis_deci_celsius,
        .tiempo_minimo_encendido_ms = parametros->tiempo_minimo_encendido_ms,
        .tiempo_minimo_apagado_ms = parametros->tiempo_minimo_apagado_ms,
    });

    if (!temperatura_valida) {
        estado_hmi.salida_activa = false;
        estado_hmi.sensor_disponible = false;
        hmi_cargar_estado_proceso(&estado_hmi);
        led_turn_off(LED1);
        return;
    }

    control_on_off_procesar(temperatura_deci_celsius, APP_LOOP_DELTA_MS);
    salida_activa = control_on_off_esta_salida_activa();
    estado_hmi.salida_activa = salida_activa;
    estado_hmi.sensor_disponible = true;
    hmi_cargar_estado_proceso(&estado_hmi);

    if (salida_activa) {
        led_turn_on(LED1);
    } else {
        led_turn_off(LED1);
    }
}

void app_init(void)
{
    const parametros_control_t* parametros = 0;
    control_on_off_configuracion_t configuracion_control = {0};

    // Inicializacion de la base de tiempo y drivers discretos.
    driver_delay_init();
    board_timer_init(APP_TIMER_TICK_MS);
    led_init();
    buzzer_init();
    buzzer_turn_off();
    buttons_init();
    driver_lcd_init();

    // Inicializacion de la persistencia de parametros.
    (void) driver_eeprom_init();
    (void) parametros_init();

    // Inicializacion del sensor de temperatura.
    app_sensores_.inicializado = ds18b20_init(&app_sensores_.sensor_temperatura, &app_pin_ds18b20_);
    if (app_sensores_.inicializado) {
        (void) ds18b20_start_conversion(&app_sensores_.sensor_temperatura);
    }

    // Inicializacion de la HMI con el estado persistido.
    hmi_init();
    parametros = parametros_obtener();
    hmi_cargar_estado_proceso(&(hmi_estado_proceso_t) {0});
    hmi_cargar_parametros_control(&(hmi_parametros_control_t) {
        .setpoint_deci_celsius = parametros->setpoint_deci_celsius,
        .histeresis_deci_celsius = parametros->histeresis_deci_celsius,
        .tiempo_minimo_encendido_ms = parametros->tiempo_minimo_encendido_ms,
        .tiempo_minimo_apagado_ms = parametros->tiempo_minimo_apagado_ms,
        .modo_calentar = parametros->modo_calentar,
    });

    // Inicializacion del lazo de control a partir de los parametros cargados.
    configuracion_control.sentido = parametros->modo_calentar
        ? CONTROL_ON_OFF_SENTIDO_CALENTAR
        : CONTROL_ON_OFF_SENTIDO_ENFRIAR;
    configuracion_control.setpoint_deci_celsius = parametros->setpoint_deci_celsius;
    configuracion_control.histeresis_deci_celsius = parametros->histeresis_deci_celsius;
    configuracion_control.tiempo_minimo_encendido_ms = parametros->tiempo_minimo_encendido_ms;
    configuracion_control.tiempo_minimo_apagado_ms = parametros->tiempo_minimo_apagado_ms;
    control_on_off_inicializar(configuracion_control);

    // Arranque del lazo cooperativo temporizado.
    app_ultimo_tick_procesado_ms_ = board_timer_get_ticks();
}

void app_process(void)
{
    const uint32_t tick_actual_ms = board_timer_get_ticks();

    while ((uint32_t) (tick_actual_ms - app_ultimo_tick_procesado_ms_) >= APP_LOOP_DELTA_MS) {
        app_ultimo_tick_procesado_ms_ += APP_LOOP_DELTA_MS;

        app_step_20ms();
    }
}
