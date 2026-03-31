/**
 * @file hmi.c
 * @brief HMI simple basada en FSM para LCD 16x2 y 4 teclas.
 */

#include "hmi/hmi.h"

#include "drivers/buttons_driver.h"
#include "drivers/buzzer_driver.h"
#include "drivers/lcd_driver.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HMI_LCD_COLUMNAS 16U
#define HMI_TICK_MS 20U
#define HMI_BEEP_MS 20U
#define HMI_BEEP_TICKS (HMI_BEEP_MS / HMI_TICK_MS)

typedef enum {
    HMI_PANTALLA_INICIO = 0,
    HMI_PANTALLA_MENU,
    HMI_PANTALLA_EDICION,
} hmi_pantalla_t;

typedef enum {
    HMI_PARAM_SETPOINT = 0,
    HMI_PARAM_HISTERESIS,
    HMI_PARAM_TMIN_ON,
    HMI_PARAM_TMIN_OFF,
    HMI_PARAM_MODO,
    HMI_PARAM_COUNT,
} hmi_parametro_t;

typedef enum {
    HMI_EVENTO_NINGUNO = 0,
    HMI_EVENTO_MENU,
    HMI_EVENTO_SUBIR,
    HMI_EVENTO_BAJAR,
    HMI_EVENTO_ACEPTAR,
} hmi_evento_t;

typedef struct {
    const char* titulo;
    int16_t minimo;
    int16_t maximo;
    int16_t paso;
    bool ciclico;
} hmi_param_desc_t;

typedef struct {
    bool temperatura_valida;
    int16_t temperatura_deci_celsius;
    bool salida_activa;
    bool sensor_disponible;
    hmi_pantalla_t pantalla;
    hmi_parametro_t menu_index;
    hmi_parametro_t editando;
    int16_t valor_edicion;
    bool necesita_redibujado;
    uint8_t ticks_buzzer_restantes;
} hmi_ui_t;

typedef struct {
    parametros_control_t parametros;
    hmi_ui_t ui;
} hmi_estado_t;

static hmi_estado_t hmi_ = {
    .parametros = {
        .setpoint_deci_celsius = 270,
        .histeresis_deci_celsius = 20U,
        .tiempo_minimo_encendido_ms = 0U,
        .tiempo_minimo_apagado_ms = 0U,
        .modo_calentar = true,
    },
    .ui = {
        .pantalla = HMI_PANTALLA_INICIO,
        .menu_index = HMI_PARAM_SETPOINT,
        .editando = HMI_PARAM_SETPOINT,
        .valor_edicion = 0,
        .necesita_redibujado = true,
        .ticks_buzzer_restantes = 0U,
    },
};

static const hmi_param_desc_t hmi_param_descs_[HMI_PARAM_COUNT] = {
    [HMI_PARAM_SETPOINT] = {.titulo = "Setpoint", .minimo = 0, .maximo = 1200, .paso = 1, .ciclico = false},
    [HMI_PARAM_HISTERESIS] = {.titulo = "Histeresis", .minimo = 1, .maximo = 200, .paso = 1, .ciclico = false},
    [HMI_PARAM_TMIN_ON] = {.titulo = "Tmin ON", .minimo = 0, .maximo = 6000, .paso = 1, .ciclico = false},
    [HMI_PARAM_TMIN_OFF] = {.titulo = "Tmin OFF", .minimo = 0, .maximo = 6000, .paso = 1, .ciclico = false},
    [HMI_PARAM_MODO] = {.titulo = "Modo", .minimo = 0, .maximo = 1, .paso = 1, .ciclico = true},
};

static void hmi_escribir_linea(uint8_t fila, const char* texto)
{
    char linea[HMI_LCD_COLUMNAS + 1U];

    (void) snprintf(linea, sizeof(linea), "%-*.*s", HMI_LCD_COLUMNAS, HMI_LCD_COLUMNAS, texto);
    driver_lcd_set_position(1U, fila);
    driver_lcd_printf(linea);
}

static void hmi_formatear_deci(char* salida, size_t tam_salida, int16_t valor_deci)
{
    const bool negativo = (valor_deci < 0);
    const int16_t absoluto = (int16_t) abs(valor_deci);
    const int16_t entera = (int16_t) (absoluto / 10);
    const int16_t decimal = (int16_t) (absoluto % 10);

    if (negativo) {
        (void) snprintf(salida, tam_salida, "-%d.%1d", entera, decimal);
    } else {
        (void) snprintf(salida, tam_salida, "%d.%1d", entera, decimal);
    }
}

static int16_t hmi_cargar_valor_edicion_actual(void)
{
    // Convierte el parametro seleccionado a la representacion entera usada en la pantalla de edicion.
    switch (hmi_.ui.editando) {
    case HMI_PARAM_SETPOINT:
        return hmi_.parametros.setpoint_deci_celsius;
    case HMI_PARAM_HISTERESIS:
        return (int16_t) hmi_.parametros.histeresis_deci_celsius;
    case HMI_PARAM_TMIN_ON:
        return (int16_t) (hmi_.parametros.tiempo_minimo_encendido_ms / 100U);
    case HMI_PARAM_TMIN_OFF:
        return (int16_t) (hmi_.parametros.tiempo_minimo_apagado_ms / 100U);
    case HMI_PARAM_MODO:
    default:
        return hmi_.parametros.modo_calentar ? 1 : 0;
    }
}

static void hmi_guardar_valor_editado(void)
{
    // Lleva el valor editado de vuelta al parametro persistible correspondiente.
    switch (hmi_.ui.editando) {
    case HMI_PARAM_SETPOINT:
        hmi_.parametros.setpoint_deci_celsius = hmi_.ui.valor_edicion;
        break;
    case HMI_PARAM_HISTERESIS:
        hmi_.parametros.histeresis_deci_celsius = (uint16_t) hmi_.ui.valor_edicion;
        break;
    case HMI_PARAM_TMIN_ON:
        hmi_.parametros.tiempo_minimo_encendido_ms = (uint32_t) hmi_.ui.valor_edicion * 100U;
        break;
    case HMI_PARAM_TMIN_OFF:
        hmi_.parametros.tiempo_minimo_apagado_ms = (uint32_t) hmi_.ui.valor_edicion * 100U;
        break;
    case HMI_PARAM_MODO:
        hmi_.parametros.modo_calentar = (hmi_.ui.valor_edicion != 0);
        break;
    default:
        break;
    }
}

static void hmi_dibujar_inicio(void)
{
    char temp_con_unidad[8];
    char sp[8];
    char h[8];
    char linea1[HMI_LCD_COLUMNAS + 1U];
    char linea2[HMI_LCD_COLUMNAS + 1U];
    const char* salida = hmi_.ui.salida_activa ? "ON" : "OFF";
    const char* modo = hmi_.parametros.modo_calentar ? "CAL" : "ENF";

    if (hmi_.ui.sensor_disponible && hmi_.ui.temperatura_valida) {
        char temp[8];
        hmi_formatear_deci(temp, sizeof(temp), hmi_.ui.temperatura_deci_celsius);
        (void) snprintf(temp_con_unidad, sizeof(temp_con_unidad), "%sC", temp);
    } else {
        (void) snprintf(temp_con_unidad, sizeof(temp_con_unidad), "--.-C");
    }

    hmi_formatear_deci(sp, sizeof(sp), hmi_.parametros.setpoint_deci_celsius);
    hmi_formatear_deci(h, sizeof(h), (int16_t) hmi_.parametros.histeresis_deci_celsius);

    (void) snprintf(linea1, sizeof(linea1), "T:%s %s %s", temp_con_unidad, salida, modo);
    (void) snprintf(linea2, sizeof(linea2), "SP:%s H:%s", sp, h);

    hmi_escribir_linea(1U, linea1);
    hmi_escribir_linea(2U, linea2);
}

static void hmi_dibujar_menu(void)
{
    hmi_escribir_linea(1U, "MENU:Param control");
    hmi_escribir_linea(2U, hmi_param_descs_[hmi_.ui.menu_index].titulo);
}

static void hmi_dibujar_edicion(void)
{
    char valor[12];
    char linea2[HMI_LCD_COLUMNAS + 1U];
    const hmi_parametro_t parametro = hmi_.ui.editando;

    hmi_escribir_linea(1U, "EDITAR");

    if (parametro == HMI_PARAM_MODO) {
        (void) snprintf(
            linea2,
            sizeof(linea2),
            "Modo:%s",
            (hmi_.ui.valor_edicion != 0) ? "Calentar" : "Enfriar"
        );
        hmi_escribir_linea(2U, linea2);
        return;
    }

    hmi_formatear_deci(valor, sizeof(valor), hmi_.ui.valor_edicion);

    if ((parametro == HMI_PARAM_TMIN_ON) || (parametro == HMI_PARAM_TMIN_OFF)) {
        (void) snprintf(linea2, sizeof(linea2), "%s:%ss", hmi_param_descs_[parametro].titulo, valor);
    } else {
        (void) snprintf(linea2, sizeof(linea2), "%s:%s", hmi_param_descs_[parametro].titulo, valor);
    }

    hmi_escribir_linea(2U, linea2);
}

static void hmi_dibujar(void)
{
    if (!hmi_.ui.necesita_redibujado) {
        return;
    }

    switch (hmi_.ui.pantalla) {
    case HMI_PANTALLA_INICIO:
        hmi_dibujar_inicio();
        break;
    case HMI_PANTALLA_MENU:
        hmi_dibujar_menu();
        break;
    case HMI_PANTALLA_EDICION:
        hmi_dibujar_edicion();
        break;
    default:
        break;
    }

    hmi_.ui.necesita_redibujado = false;
}

void hmi_init(void)
{
    hmi_.ui.pantalla = HMI_PANTALLA_INICIO;
    hmi_.ui.menu_index = HMI_PARAM_SETPOINT;
    hmi_.ui.editando = HMI_PARAM_SETPOINT;
    hmi_.ui.valor_edicion = 0;
    hmi_.ui.necesita_redibujado = true;
    hmi_.ui.ticks_buzzer_restantes = 0U;

    driver_lcd_write_char('\b');
    hmi_dibujar();
}

void hmi_process(void)
{
    hmi_evento_t evento = HMI_EVENTO_NINGUNO;
    const uint8_t tecla = button_get_event();

    if (tecla == TECLA1) {
        evento = HMI_EVENTO_MENU;
    } else if (tecla == TECLA2) {
        evento = HMI_EVENTO_SUBIR;
    } else if (tecla == TECLA3) {
        evento = HMI_EVENTO_BAJAR;
    } else if (tecla == TECLA4) {
        evento = HMI_EVENTO_ACEPTAR;
    }

    if (hmi_.ui.ticks_buzzer_restantes > 0U) {
        hmi_.ui.ticks_buzzer_restantes--;
        if (hmi_.ui.ticks_buzzer_restantes == 0U) {
            buzzer_turn_off();
        }
    }

    if (evento != HMI_EVENTO_NINGUNO) {
        hmi_.ui.ticks_buzzer_restantes = HMI_BEEP_TICKS;
        buzzer_turn_on();
    }

    switch (hmi_.ui.pantalla) {
    case HMI_PANTALLA_INICIO:
        if (evento == HMI_EVENTO_MENU) {
            hmi_.ui.pantalla = HMI_PANTALLA_MENU;
            hmi_.ui.necesita_redibujado = true;
        }
        break;

    case HMI_PANTALLA_MENU:
        if (evento == HMI_EVENTO_MENU) {
            hmi_.ui.pantalla = HMI_PANTALLA_INICIO;
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_SUBIR) {
            if (hmi_.ui.menu_index == 0U) {
                hmi_.ui.menu_index = (hmi_parametro_t) (HMI_PARAM_COUNT - 1U);
            } else {
                hmi_.ui.menu_index = (hmi_parametro_t) (hmi_.ui.menu_index - 1U);
            }
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_BAJAR) {
            hmi_.ui.menu_index = (hmi_parametro_t) ((hmi_.ui.menu_index + 1U) % HMI_PARAM_COUNT);
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_ACEPTAR) {
            hmi_.ui.editando = hmi_.ui.menu_index;
            hmi_.ui.valor_edicion = hmi_cargar_valor_edicion_actual();
            hmi_.ui.pantalla = HMI_PANTALLA_EDICION;
            hmi_.ui.necesita_redibujado = true;
        }
        break;

    case HMI_PANTALLA_EDICION:
        if (evento == HMI_EVENTO_MENU) {
            hmi_.ui.pantalla = HMI_PANTALLA_MENU;
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_SUBIR) {
            const hmi_param_desc_t* desc = &hmi_param_descs_[hmi_.ui.editando];
            hmi_.ui.valor_edicion = (int16_t) (hmi_.ui.valor_edicion + desc->paso);
            // Limita el valor editado al rango permitido y hace wrap solo en Modo.
            if (hmi_.ui.valor_edicion > desc->maximo) {
                hmi_.ui.valor_edicion = desc->ciclico ? desc->minimo : desc->maximo;
            }
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_BAJAR) {
            const hmi_param_desc_t* desc = &hmi_param_descs_[hmi_.ui.editando];
            hmi_.ui.valor_edicion = (int16_t) (hmi_.ui.valor_edicion - desc->paso);
            // Limita el valor editado al rango permitido y hace wrap solo en Modo.
            if (hmi_.ui.valor_edicion < desc->minimo) {
                hmi_.ui.valor_edicion = desc->ciclico ? desc->maximo : desc->minimo;
            }
            hmi_.ui.necesita_redibujado = true;
        } else if (evento == HMI_EVENTO_ACEPTAR) {
            hmi_guardar_valor_editado();
            hmi_.ui.pantalla = HMI_PANTALLA_MENU;
            hmi_.ui.necesita_redibujado = true;
        }
        break;

    default:
        break;
    }

    hmi_dibujar();
}

void hmi_cargar_parametros_control(const parametros_control_t* parametros)
{
    if (parametros == 0) {
        return;
    }

    hmi_.parametros = *parametros;
    hmi_.ui.necesita_redibujado = true;
}

void hmi_cargar_estado_proceso(const hmi_estado_proceso_t* estado)
{
    const bool hubo_cambios = (estado != 0)
        && ((hmi_.ui.temperatura_valida != estado->temperatura_valida)
            || (hmi_.ui.temperatura_deci_celsius != estado->temperatura_deci_celsius)
            || (hmi_.ui.salida_activa != estado->salida_activa)
            || (hmi_.ui.sensor_disponible != estado->sensor_disponible));

    if (estado == 0) {
        return;
    }

    hmi_.ui.temperatura_valida = estado->temperatura_valida;
    hmi_.ui.temperatura_deci_celsius = estado->temperatura_deci_celsius;
    hmi_.ui.salida_activa = estado->salida_activa;
    hmi_.ui.sensor_disponible = estado->sensor_disponible;

    if (hubo_cambios && (hmi_.ui.pantalla == HMI_PANTALLA_INICIO)) {
        hmi_.ui.necesita_redibujado = true;
    }
}

parametros_control_t hmi_obtener_parametros_control(void)
{
    return hmi_.parametros;
}
