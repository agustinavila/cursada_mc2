/**
 * @file app.c
 * @brief Implementacion de la capa principal de aplicacion.
 */

#include "app/app.h"

#include "Driver/adc_driver.h"
#include "Driver/buttons_driver.h"
#include "Driver/buzzer_driver.h"
#include "Driver/eeprom_driver.h"
#include "Driver/keyboard_driver.h"
#include "Driver/lcd_driver.h"
#include "Driver/led_Driver.h"
#include "app/parametros.h"
#include "control/control.h"
#include "control/control_selector.h"
#include "control/control_on_off.h"
#include "hmi/hmi.h"

#include <string.h>

static control_selector_t app_selector_control_;
static control_on_off_configuracion_t app_control_on_off_configuracion_actual_;
static uint8_t app_indice_sensor_proceso_ = 0U;
static bool app_sensor_proceso_resuelto_ = false;

/**
 * @brief Copia en la HMI los parametros persistidos actualmente cargados.
 *
 * La HMI sigue siendo la interfaz de edicion, pero la fuente de verdad queda
 * en el modulo de parametros para poder persistir entre reinicios y reflasheos.
 */
static void app_cargar_parametros_en_hmi(void)
{
    const parametros_t* parametros = parametros_obtener();
    bool sensor_automatico = true;
    uint8_t seleccion_sensor = 0U;

    hmi_cargar_parametros_control(parametros->control.setpoint_deci_celsius,
                                  parametros->control.histeresis_deci_celsius,
                                  parametros->control.modo_calentar);

    if ((hmi_obtener_cantidad_sensores() > 1U)
        && (parametros->sensor_proceso.modo == PARAMETROS_SENSOR_MODO_ROM)
        && parametros->sensor_proceso.rom_valida) {
        sensor_automatico = false;
        uint8_t indice = 0U;
        uint8_t rom_sensor[PARAMETROS_SENSOR_ROM_SIZE];

        for (indice = 0U; indice < hmi_obtener_cantidad_sensores(); ++indice) {
            if (hmi_obtener_rom_sensor(indice, rom_sensor)
                && (memcmp(rom_sensor, parametros->sensor_proceso.rom, PARAMETROS_SENSOR_ROM_SIZE) == 0)) {
                seleccion_sensor = (uint8_t) (indice + 1U);
                break;
            }
        }
    }

    hmi_cargar_configuracion_sensor_proceso(sensor_automatico, seleccion_sensor);
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
        .habilitado = true,
    };

    return configuracion;
}

/**
 * @brief Resuelve que sensor descubierto debe usarse como entrada del lazo.
 *
 * Politica actual:
 * - sin sensores: no hay control posible
 * - un solo sensor: se usa automaticamente
 * - multiples sensores: solo se acepta una ROM persistida valida
 */
static bool app_resolver_sensor_proceso(void)
{
    const parametros_t* parametros = parametros_obtener();
    const uint8_t cantidad_sensores = hmi_obtener_cantidad_sensores();
    uint8_t indice = 0U;
    uint8_t rom_sensor[PARAMETROS_SENSOR_ROM_SIZE];

    app_sensor_proceso_resuelto_ = false;

    if (cantidad_sensores == 0U) {
        return false;
    }

    if (cantidad_sensores == 1U) {
        app_indice_sensor_proceso_ = 0U;
        app_sensor_proceso_resuelto_ = true;
        return true;
    }

    if ((parametros->sensor_proceso.modo != PARAMETROS_SENSOR_MODO_ROM)
        || !parametros->sensor_proceso.rom_valida) {
        return false;
    }

    for (indice = 0U; indice < cantidad_sensores; ++indice) {
        if (hmi_obtener_rom_sensor(indice, rom_sensor)
            && (memcmp(rom_sensor, parametros->sensor_proceso.rom, PARAMETROS_SENSOR_ROM_SIZE) == 0)) {
            app_indice_sensor_proceso_ = indice;
            app_sensor_proceso_resuelto_ = true;
            return true;
        }
    }

    return false;
}

/**
 * @brief Resincroniza el control activo cuando cambian los parametros.
 *
 * Se evita reconfigurar el controlador si la configuracion efectiva no cambio.
 */
static bool app_sincronizar_control_desde_parametros(void)
{
    control_generico_t* control_activo = 0;
    control_on_off_configuracion_t nueva_configuracion = app_obtener_configuracion_control_desde_parametros();

    control_activo = control_selector_obtener_activo(&app_selector_control_);
    if (control_activo == 0) {
        return false;
    }

    if ((nueva_configuracion.setpoint_deci_celsius == app_control_on_off_configuracion_actual_.setpoint_deci_celsius)
        && (nueva_configuracion.histeresis_deci_celsius == app_control_on_off_configuracion_actual_.histeresis_deci_celsius)
        && (nueva_configuracion.sentido == app_control_on_off_configuracion_actual_.sentido)
        && (nueva_configuracion.habilitado == app_control_on_off_configuracion_actual_.habilitado)) {
        return true;
    }

    app_control_on_off_configuracion_actual_ = nueva_configuracion;
    return control_on_off_configurar((control_on_off_t*) control_activo,
                                     &app_control_on_off_configuracion_actual_);
}

/**
 * @brief Lleva a la copia persistente los cambios ya confirmados en la HMI.
 *
 * La HMI solo actualiza sus variables internas al presionar Enter. Una vez que
 * eso ocurre, la app refleja los nuevos valores en RAM y los guarda en EEPROM.
 */
static bool app_sincronizar_hmi_en_parametros(void)
{
    const int16_t setpoint_deci_celsius = hmi_obtener_setpoint_deci_celsius();
    const uint16_t histeresis_deci_celsius = hmi_obtener_histeresis_deci_celsius();
    const bool modo_calentar = hmi_modo_control_es_calentar();
    const bool sensor_automatico = hmi_sensor_proceso_es_automatico();
    const uint8_t seleccion_sensor = hmi_obtener_sensor_proceso_seleccion();
    const uint8_t cantidad_sensores = hmi_obtener_cantidad_sensores();
    uint8_t rom_sensor[PARAMETROS_SENSOR_ROM_SIZE];
    bool hubo_cambios = false;

    if (parametros_actualizar_control(setpoint_deci_celsius,
                                      histeresis_deci_celsius,
                                      modo_calentar)) {
        hubo_cambios = true;
    }

    if (sensor_automatico || (cantidad_sensores <= 1U)) {
        if (parametros_configurar_sensor_proceso_auto()) {
            hubo_cambios = true;
        }
    } else if ((seleccion_sensor <= cantidad_sensores)
               && hmi_obtener_rom_sensor((uint8_t) (seleccion_sensor - 1U), rom_sensor)) {
        if (parametros_configurar_sensor_proceso_por_rom(rom_sensor)) {
            hubo_cambios = true;
        }
    }

    if (!hubo_cambios) {
        return true;
    }

    return parametros_guardar();
}

/**
 * @brief Ejecuta el ciclo principal del lazo de control de temperatura.
 *
 * La medicion siempre se toma del sensor de proceso fijo, independiente de la
 * rotacion visual que haga la HMI sobre otros sensores disponibles.
 */
static void app_actualizar_control(void)
{
    control_generico_t* control_activo = 0;
    int16_t temperatura_deci_celsius = 0;

    control_activo = control_selector_obtener_activo(&app_selector_control_);

    if ((control_activo == 0)
        || !app_resolver_sensor_proceso()
        || !hmi_obtener_temperatura_sensor(app_indice_sensor_proceso_, &temperatura_deci_celsius)) {
        led_turn_off(LED1);
        return;
    }

    if (!app_sincronizar_hmi_en_parametros() || !app_sincronizar_control_desde_parametros()) {
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
    (void) driver_eeprom_init();
    (void) parametros_init();
    hmi_init();
    app_cargar_parametros_en_hmi();
    app_control_on_off_configuracion_actual_ = app_obtener_configuracion_control_desde_parametros();
    (void) control_selector_inicializar(&app_selector_control_);
    (void) control_selector_seleccionar_on_off(&app_selector_control_,
                                               &app_control_on_off_configuracion_actual_);
}

void app_process(void)
{
    hmi_process();
    if (hmi_consumir_solicitud_restablecer_parametros()) {
        /* El restablecimiento repone defaults en RAM, EEPROM, HMI y control. */
        parametros_restablecer_defaults();
        app_cargar_parametros_en_hmi();
    }
    app_actualizar_control();
}
