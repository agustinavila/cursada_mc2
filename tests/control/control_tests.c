#include "test_harness.h"

#include "control/control_on_off.h"

static control_on_off_configuracion_t control_crear_configuracion(control_on_off_sentido_t sentido,
                                                                  int16_t setpoint,
                                                                  uint16_t histeresis,
                                                                  uint32_t tiempo_minimo_encendido_ms,
                                                                  uint32_t tiempo_minimo_apagado_ms,
                                                                  bool habilitado)
{
    control_on_off_configuracion_t configuracion = {
        .sentido = sentido,
        .setpoint_deci_celsius = setpoint,
        .histeresis_deci_celsius = histeresis,
        .tiempo_minimo_encendido_ms = tiempo_minimo_encendido_ms,
        .tiempo_minimo_apagado_ms = tiempo_minimo_apagado_ms,
        .habilitado = habilitado,
    };

    return configuracion;
}

static control_on_off_t control_on_off_crear_vacio(void)
{
    control_on_off_t control = {
        .configuracion = { 0 },
        .tiempo_en_estado_ms = 0U,
        .salida_activa = false,
        .inicializado = false,
        .tiene_medicion = false,
        .ultima_medicion_deci_celsius = 0,
    };

    return control;
}

static void test_control_on_off_inicializacion_y_configuracion(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 200U, 300U, true);
    control_on_off_configuracion_t leida = { 0 };

    TEST_ASSERT_FALSE(control_on_off_inicializar(0, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_inicializar(&control, 0));

    configuracion.sentido = (control_on_off_sentido_t) 99;
    TEST_ASSERT_FALSE(control_on_off_inicializar(&control, &configuracion));

    configuracion = control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 200U, 300U, true);
    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_obtener_configuracion(&control, &leida));
    TEST_ASSERT_EQ_INT(configuracion.setpoint_deci_celsius, leida.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(configuracion.histeresis_deci_celsius, leida.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(configuracion.sentido, leida.sentido);
    TEST_ASSERT_EQ_UINT(configuracion.tiempo_minimo_encendido_ms, leida.tiempo_minimo_encendido_ms);
    TEST_ASSERT_EQ_UINT(configuracion.tiempo_minimo_apagado_ms, leida.tiempo_minimo_apagado_ms);
    TEST_ASSERT_EQ_BOOL(configuracion.habilitado, leida.habilitado);
}

static void test_control_on_off_configurar_valida_errores_y_reinicia_estado(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 10U, 15U, true);
    control_on_off_configuracion_t invalida = configuracion;
    int16_t medicion = 0;

    TEST_ASSERT_FALSE(control_on_off_configurar(0, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_configurar(&control, &configuracion));

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_obtener_ultima_medicion(&control, &medicion));
    TEST_ASSERT_EQ_INT(250, medicion);

    invalida.sentido = (control_on_off_sentido_t) 99;
    TEST_ASSERT_FALSE(control_on_off_configurar(&control, &invalida));

    configuracion = control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_ENFRIAR, 300, 30U, 0U, 0U, true);
    TEST_ASSERT_TRUE(control_on_off_configurar(&control, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(&control, &medicion));
}

static void test_control_on_off_calentar_respeta_histeresis_y_corte(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 0U, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 251, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 260, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_mantiene_estado_dentro_de_la_banda(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 0U, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 261, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_enfriar_respeta_histeresis_y_corte(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_ENFRIAR, 270, 20U, 0U, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 289, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 290, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 280, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_enfriar_mantiene_estado_dentro_de_la_banda(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_ENFRIAR, 270, 20U, 0U, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 290, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 279, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_deshabilitado_y_reinicio(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 0U, 0U, false);
    int16_t medicion = 0;

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 200, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_obtener_ultima_medicion(&control, &medicion));
    TEST_ASSERT_EQ_INT(200, medicion);

    control_on_off_reiniciar(&control);
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(&control, &medicion));
}

static void test_control_on_off_api_rechaza_instancias_invalidas(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    control_on_off_configuracion_t configuracion = { 0 };
    int16_t medicion = 0;

    TEST_ASSERT_FALSE(control_on_off_procesar(0, 250, 20U));
    TEST_ASSERT_FALSE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(0));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(0, &medicion));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(&control, &medicion));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(&control, 0));
    TEST_ASSERT_FALSE(control_on_off_obtener_configuracion(0, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_obtener_configuracion(&control, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_obtener_configuracion(&control, 0));

    control_on_off_reiniciar(0);
    control_on_off_reiniciar(&control);
}

static void test_control_on_off_respeta_tiempo_minimo_apagado(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 0U, 60U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_respeta_tiempo_minimo_encendido(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 40U, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, 20U));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_satura_tiempo_en_estado(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, UINT32_MAX, 0U, true);

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 250, UINT32_MAX));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    control.tiempo_en_estado_ms = UINT32_MAX - 5U;
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 260, 10U));
    TEST_ASSERT_TRUE(control_on_off_esta_salida_activa(&control));

    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 270, UINT32_MAX));
    TEST_ASSERT_FALSE(control_on_off_esta_salida_activa(&control));
}

static void test_control_on_off_expone_ultima_medicion(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, 0U, 0U, true);
    int16_t medicion = 0;

    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(0, &medicion));
    TEST_ASSERT_FALSE(control_on_off_obtener_ultima_medicion(&control, &medicion));
    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_procesar(&control, 263, 20U));
    TEST_ASSERT_TRUE(control_on_off_obtener_ultima_medicion(&control, &medicion));
    TEST_ASSERT_EQ_INT(263, medicion);
}

int main(void)
{
    test_harness_reset();

    RUN_TEST(test_control_on_off_inicializacion_y_configuracion);
    RUN_TEST(test_control_on_off_configurar_valida_errores_y_reinicia_estado);
    RUN_TEST(test_control_on_off_calentar_respeta_histeresis_y_corte);
    RUN_TEST(test_control_on_off_mantiene_estado_dentro_de_la_banda);
    RUN_TEST(test_control_on_off_enfriar_respeta_histeresis_y_corte);
    RUN_TEST(test_control_on_off_enfriar_mantiene_estado_dentro_de_la_banda);
    RUN_TEST(test_control_on_off_deshabilitado_y_reinicio);
    RUN_TEST(test_control_on_off_api_rechaza_instancias_invalidas);
    RUN_TEST(test_control_on_off_respeta_tiempo_minimo_apagado);
    RUN_TEST(test_control_on_off_respeta_tiempo_minimo_encendido);
    RUN_TEST(test_control_on_off_satura_tiempo_en_estado);
    RUN_TEST(test_control_on_off_expone_ultima_medicion);

    return test_harness_result();
}
