#include "test_harness.h"

#include "app/parametros.h"
#include "app/parametros_default.h"
#include "fake_eeprom_driver.h"

static void test_parametros_carga_defaults_si_eeprom_falla(void)
{
    const parametros_t* parametros = 0;
    const parametros_t* defaults = parametros_default_obtener();

    fake_eeprom_reset();
    fake_eeprom_set_read_ok(false);

    TEST_ASSERT_TRUE(parametros_init());
    parametros = parametros_obtener();

    TEST_ASSERT_EQ_INT(defaults->control.setpoint_deci_celsius, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(defaults->control.histeresis_deci_celsius, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(defaults->control.tiempo_minimo_encendido_ms, parametros->control.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(defaults->control.tiempo_minimo_apagado_ms, parametros->control.tiempo_minimo_apagado_ms);
    TEST_ASSERT_EQ_BOOL(defaults->control.modo_calentar, parametros->control.modo_calentar);
}

static void test_parametros_carga_defaults_si_eeprom_es_invalida(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_apagado_ms);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

static void test_parametros_guardar_y_recuperar_control(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(315, 35U, 1200U, 800U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    TEST_ASSERT_TRUE(parametros_actualizar_control(100, 10U, 200U, 100U, true));
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(315, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(35U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(1200U, parametros->control.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(800U, parametros->control.tiempo_minimo_apagado_ms);
    TEST_ASSERT_FALSE(parametros->control.modo_calentar);
}

static void test_parametros_restablecer_defaults_persiste(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(320, 25U, 900U, 400U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    parametros_restablecer_defaults();
    TEST_ASSERT_TRUE(parametros_actualizar_control(190, 10U, 100U, 50U, false));
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_apagado_ms);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

static void test_parametros_detecta_corrupcion_persistida(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(300, 15U, 500U, 250U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    fake_eeprom_corrupt_byte(0U, 0U);
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(0U, parametros->control.tiempo_minimo_apagado_ms);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

int main(void)
{
    test_harness_reset();

    RUN_TEST(test_parametros_carga_defaults_si_eeprom_falla);
    RUN_TEST(test_parametros_carga_defaults_si_eeprom_es_invalida);
    RUN_TEST(test_parametros_guardar_y_recuperar_control);
    RUN_TEST(test_parametros_restablecer_defaults_persiste);
    RUN_TEST(test_parametros_detecta_corrupcion_persistida);

    return test_harness_result();
}
