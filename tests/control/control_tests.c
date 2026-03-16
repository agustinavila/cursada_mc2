#include "test_harness.h"

#include "control/control.h"
#include "control/control_on_off.h"
#include "control/control_selector.h"

typedef struct {
    control_generico_t base;
    bool salida_activa;
    bool tiene_medicion;
    int16_t ultima_medicion;
    unsigned reinicios;
    unsigned procesamientos;
} control_stub_t;

static void control_stub_reiniciar(control_generico_t* base)
{
    control_stub_t* control = (control_stub_t*) base;

    control->salida_activa = false;
    control->tiene_medicion = false;
    control->ultima_medicion = 0;
    ++control->reinicios;
}

static bool control_stub_procesar(control_generico_t* base, int16_t medicion)
{
    control_stub_t* control = (control_stub_t*) base;

    control->ultima_medicion = medicion;
    control->tiene_medicion = true;
    control->salida_activa = (medicion >= 0);
    ++control->procesamientos;
    return true;
}

static bool control_stub_salida_activa(const control_generico_t* base)
{
    const control_stub_t* control = (const control_stub_t*) base;

    return control->salida_activa;
}

static bool control_stub_obtener_ultima_medicion(const control_generico_t* base, int16_t* medicion)
{
    const control_stub_t* control = (const control_stub_t*) base;

    if ((medicion == 0) || !control->tiene_medicion) {
        return false;
    }

    *medicion = control->ultima_medicion;
    return true;
}

static control_operaciones_t control_stub_operaciones_ = {
    .reiniciar = control_stub_reiniciar,
    .procesar = control_stub_procesar,
    .salida_activa = control_stub_salida_activa,
    .obtener_ultima_medicion = control_stub_obtener_ultima_medicion,
};

static control_on_off_configuracion_t control_crear_configuracion(control_on_off_sentido_t sentido,
                                                                  int16_t setpoint,
                                                                  uint16_t histeresis,
                                                                  bool habilitado)
{
    control_on_off_configuracion_t configuracion = {
        .sentido = sentido,
        .setpoint_deci_celsius = setpoint,
        .histeresis_deci_celsius = histeresis,
        .habilitado = habilitado,
    };

    return configuracion;
}

static control_on_off_t control_on_off_crear_vacio(void)
{
    control_on_off_t control = {
        .base = { 0 },
        .configuracion = { 0 },
        .salida_activa = false,
        .inicializado = false,
        .tiene_medicion = false,
        .ultima_medicion_deci_celsius = 0,
    };

    return control;
}

static void test_control_generico_rechaza_nulos(void)
{
    int16_t medicion = 0;

    control_reiniciar(0);
    TEST_ASSERT_FALSE(control_procesar(0, 123));
    TEST_ASSERT_FALSE(control_esta_salida_activa(0));
    TEST_ASSERT_FALSE(control_obtener_ultima_medicion(0, &medicion));
    TEST_ASSERT_FALSE(control_obtener_ultima_medicion(0, 0));
}

static void test_control_generico_delega_operaciones(void)
{
    control_stub_t control = {
        .base = {
            .operaciones = &control_stub_operaciones_,
        },
    };
    int16_t medicion = 0;

    TEST_ASSERT_TRUE(control_procesar(&control.base, 42));
    TEST_ASSERT_EQ_UINT(1U, control.procesamientos);
    TEST_ASSERT_TRUE(control_esta_salida_activa(&control.base));
    TEST_ASSERT_TRUE(control_obtener_ultima_medicion(&control.base, &medicion));
    TEST_ASSERT_EQ_INT(42, medicion);

    control_reiniciar(&control.base);
    TEST_ASSERT_EQ_UINT(1U, control.reinicios);
    TEST_ASSERT_FALSE(control_esta_salida_activa(&control.base));
    TEST_ASSERT_FALSE(control_obtener_ultima_medicion(&control.base, &medicion));
}

static void test_control_on_off_inicializacion_y_configuracion(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, true);
    control_on_off_configuracion_t leida = { 0 };

    TEST_ASSERT_FALSE(control_on_off_inicializar(0, &configuracion));
    TEST_ASSERT_FALSE(control_on_off_inicializar(&control, 0));

    configuracion.sentido = (control_on_off_sentido_t) 99;
    TEST_ASSERT_FALSE(control_on_off_inicializar(&control, &configuracion));

    configuracion = control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, true);
    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    TEST_ASSERT_TRUE(control_on_off_obtener_configuracion(&control, &leida));
    TEST_ASSERT_EQ_INT(configuracion.setpoint_deci_celsius, leida.setpoint_deci_celsius);
    TEST_ASSERT_EQ_UINT(configuracion.histeresis_deci_celsius, leida.histeresis_deci_celsius);
    TEST_ASSERT_EQ_UINT(configuracion.sentido, leida.sentido);
    TEST_ASSERT_EQ_BOOL(configuracion.habilitado, leida.habilitado);
}

static void test_control_on_off_calentar_respeta_histeresis_y_corte(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, true);
    control_generico_t* generico = 0;

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    generico = control_on_off_como_generico(&control);

    TEST_ASSERT_TRUE(control_procesar(generico, 251));
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 250));
    TEST_ASSERT_TRUE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 260));
    TEST_ASSERT_TRUE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 270));
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));
}

static void test_control_on_off_enfriar_respeta_histeresis_y_corte(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_ENFRIAR, 270, 20U, true);
    control_generico_t* generico = 0;

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    generico = control_on_off_como_generico(&control);

    TEST_ASSERT_TRUE(control_procesar(generico, 289));
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 290));
    TEST_ASSERT_TRUE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 280));
    TEST_ASSERT_TRUE(control_esta_salida_activa(generico));

    TEST_ASSERT_TRUE(control_procesar(generico, 270));
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));
}

static void test_control_on_off_deshabilitado_y_reinicio(void)
{
    control_on_off_t control = control_on_off_crear_vacio();
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, false);
    control_generico_t* generico = 0;
    int16_t medicion = 0;

    TEST_ASSERT_TRUE(control_on_off_inicializar(&control, &configuracion));
    generico = control_on_off_como_generico(&control);

    TEST_ASSERT_TRUE(control_procesar(generico, 200));
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));
    TEST_ASSERT_TRUE(control_obtener_ultima_medicion(generico, &medicion));
    TEST_ASSERT_EQ_INT(200, medicion);

    control_reiniciar(generico);
    TEST_ASSERT_FALSE(control_esta_salida_activa(generico));
    TEST_ASSERT_FALSE(control_obtener_ultima_medicion(generico, &medicion));
}

static void test_control_selector_selecciona_y_deshabilita(void)
{
    control_selector_t selector = { 0 };
    const control_on_off_configuracion_t configuracion =
        control_crear_configuracion(CONTROL_ON_OFF_SENTIDO_CALENTAR, 270, 20U, true);
    control_generico_t* activo = 0;

    TEST_ASSERT_FALSE(control_selector_inicializar(0));
    TEST_ASSERT_TRUE(control_selector_inicializar(&selector));
    TEST_ASSERT_EQ_UINT(CONTROL_TIPO_NINGUNO, control_selector_obtener_tipo(&selector));
    TEST_ASSERT_TRUE(control_selector_obtener_activo(&selector) == 0);

    TEST_ASSERT_FALSE(control_selector_seleccionar_on_off(0, &configuracion));
    TEST_ASSERT_FALSE(control_selector_seleccionar_on_off(&selector, 0));

    TEST_ASSERT_TRUE(control_selector_seleccionar_on_off(&selector, &configuracion));
    TEST_ASSERT_EQ_UINT(CONTROL_TIPO_ON_OFF, control_selector_obtener_tipo(&selector));

    activo = control_selector_obtener_activo(&selector);
    TEST_ASSERT_TRUE(activo != 0);
    TEST_ASSERT_TRUE(control_procesar(activo, 250));
    TEST_ASSERT_TRUE(control_esta_salida_activa(activo));

    control_selector_deshabilitar(&selector);
    TEST_ASSERT_EQ_UINT(CONTROL_TIPO_NINGUNO, control_selector_obtener_tipo(&selector));
    TEST_ASSERT_TRUE(control_selector_obtener_activo(&selector) == 0);
    TEST_ASSERT_TRUE(control_selector_obtener_activo_const(&selector) == 0);
}

int main(void)
{
    test_harness_reset();

    RUN_TEST(test_control_generico_rechaza_nulos);
    RUN_TEST(test_control_generico_delega_operaciones);
    RUN_TEST(test_control_on_off_inicializacion_y_configuracion);
    RUN_TEST(test_control_on_off_calentar_respeta_histeresis_y_corte);
    RUN_TEST(test_control_on_off_enfriar_respeta_histeresis_y_corte);
    RUN_TEST(test_control_on_off_deshabilitado_y_reinicio);
    RUN_TEST(test_control_selector_selecciona_y_deshabilita);

    return test_harness_result();
}
