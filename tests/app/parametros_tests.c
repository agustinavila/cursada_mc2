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
    TEST_ASSERT_EQ_BOOL(defaults->control.modo_calentar, parametros->control.modo_calentar);
    TEST_ASSERT_EQ_UINT(PARAMETROS_SENSOR_MODO_AUTO, parametros->sensor_proceso.modo);
    TEST_ASSERT_FALSE(parametros->sensor_proceso.rom_valida);
}

static void test_parametros_carga_defaults_si_eeprom_es_invalida(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

static void test_parametros_guardar_y_recuperar_control(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(315, 35U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    TEST_ASSERT_TRUE(parametros_actualizar_control(100, 10U, true));
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(315, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(35U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_FALSE(parametros->control.modo_calentar);
}

static void test_parametros_restablecer_defaults_persiste(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(320, 25U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    parametros_restablecer_defaults();
    TEST_ASSERT_TRUE(parametros_actualizar_control(190, 10U, false));
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

static void test_parametros_detecta_corrupcion_persistida(void)
{
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());
    TEST_ASSERT_TRUE(parametros_actualizar_control(300, 15U, false));
    TEST_ASSERT_TRUE(parametros_guardar());

    fake_eeprom_corrupt_byte(0U, 0U);
    TEST_ASSERT_TRUE(parametros_init());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_INT(270, parametros->control.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(20U, parametros->control.histeresis_deci_celsius);
    TEST_ASSERT_TRUE(parametros->control.modo_calentar);
}

static void test_parametros_configura_sensor_por_rom_y_auto(void)
{
    const uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE] = { 0x28U, 0xFFU, 0x1CU, 0x42U, 0x93U, 0x16U, 0x04U, 0x7BU };
    const uint8_t rom_nula[PARAMETROS_SENSOR_ROM_SIZE] = { 0U };
    const parametros_t* parametros = 0;

    fake_eeprom_reset();
    TEST_ASSERT_TRUE(parametros_init());

    TEST_ASSERT_FALSE(parametros_configurar_sensor_proceso_por_rom(0));
    TEST_ASSERT_TRUE(parametros_configurar_sensor_proceso_por_rom(rom));
    TEST_ASSERT_FALSE(parametros_configurar_sensor_proceso_por_rom(rom));

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_UINT(PARAMETROS_SENSOR_MODO_ROM, parametros->sensor_proceso.modo);
    TEST_ASSERT_TRUE(parametros->sensor_proceso.rom_valida);
    TEST_ASSERT_MEM_EQ(rom, parametros->sensor_proceso.rom, PARAMETROS_SENSOR_ROM_SIZE);

    TEST_ASSERT_TRUE(parametros_configurar_sensor_proceso_auto());
    TEST_ASSERT_FALSE(parametros_configurar_sensor_proceso_auto());

    parametros = parametros_obtener();
    TEST_ASSERT_EQ_UINT(PARAMETROS_SENSOR_MODO_AUTO, parametros->sensor_proceso.modo);
    TEST_ASSERT_FALSE(parametros->sensor_proceso.rom_valida);
    TEST_ASSERT_MEM_EQ(rom_nula, parametros->sensor_proceso.rom, PARAMETROS_SENSOR_ROM_SIZE);
}

int main(void)
{
    test_harness_reset();

    RUN_TEST(test_parametros_carga_defaults_si_eeprom_falla);
    RUN_TEST(test_parametros_carga_defaults_si_eeprom_es_invalida);
    RUN_TEST(test_parametros_guardar_y_recuperar_control);
    RUN_TEST(test_parametros_restablecer_defaults_persiste);
    RUN_TEST(test_parametros_detecta_corrupcion_persistida);
    RUN_TEST(test_parametros_configura_sensor_por_rom_y_auto);

    return test_harness_result();
}
