/**
 * @file hmi.c
 * @brief Implementacion simplificada de la HMI para LCD y pulsadores.
 */

#include "hmi/hmi.h"

#include "Driver/buttons_driver.h"
#include "Driver/lcd_driver.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HMI_CANTIDAD_COLUMNAS_LCD 16U

typedef enum {
    HMI_EVENTO_NINGUNO = 0,
    HMI_EVENTO_MENU,
    HMI_EVENTO_SUBIR,
    HMI_EVENTO_BAJAR,
    HMI_EVENTO_ACEPTAR,
} hmi_evento_t;

typedef enum {
    HMI_PANTALLA_INICIO = 0,
    HMI_PANTALLA_MENU,
    HMI_PANTALLA_EDICION,
} hmi_pantalla_t;

typedef enum {
    HMI_ITEM_MENU = 0,
    HMI_ITEM_PARAMETRO,
} hmi_tipo_item_t;

typedef struct {
    const char* titulo;
    uint8_t padre;
    uint8_t primer_hijo;
    uint8_t hermano_anterior;
    uint8_t hermano_siguiente;
    hmi_tipo_item_t tipo;
    int16_t* valor;
    int16_t valor_minimo;
    int16_t valor_maximo;
    int16_t paso;
} hmi_item_menu_t;

typedef struct {
    int16_t setpoint_deci_celsius;
    int16_t histeresis_deci_celsius;
    int16_t modo_calentar;
} hmi_configuracion_t;

typedef struct {
    bool temperatura_valida;
    int16_t temperatura_deci_celsius;
} hmi_sensor_t;

typedef struct {
    bool salida_activa;
    bool sensor_resuelto;
} hmi_control_t;

typedef struct {
    hmi_pantalla_t pantalla_actual;
    uint8_t nodo_actual;
    uint8_t mascara_botones_anterior;
    int16_t valor_edicion;
    bool necesita_redibujado;
} hmi_interfaz_t;

typedef struct {
    hmi_configuracion_t configuracion;
    hmi_sensor_t sensor;
    hmi_control_t control;
    hmi_interfaz_t interfaz;
} hmi_contexto_t;

enum {
    HMI_NODE_ROOT = 0,
    HMI_NODE_PARAMS,
    HMI_NODE_SETPOINT,
    HMI_NODE_HYSTERESIS,
    HMI_NODE_MODE,
};

static hmi_contexto_t hmi_ = {
    .configuracion = {
        .setpoint_deci_celsius = 270,
        .histeresis_deci_celsius = 20,
        .modo_calentar = 1,
    },
};

static const hmi_item_menu_t hmi_menu_tree_[] = {
    [HMI_NODE_ROOT] = {
        .titulo = "ROOT",
        .primer_hijo = HMI_NODE_PARAMS,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_PARAMS] = {
        .titulo = "Param control",
        .padre = HMI_NODE_ROOT,
        .primer_hijo = HMI_NODE_SETPOINT,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_SETPOINT] = {
        .titulo = "Setpoint",
        .padre = HMI_NODE_PARAMS,
        .hermano_siguiente = HMI_NODE_HYSTERESIS,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_.configuracion.setpoint_deci_celsius,
        .valor_minimo = 0,
        .valor_maximo = 1200,
        .paso = 1,
    },
    [HMI_NODE_HYSTERESIS] = {
        .titulo = "Histeresis",
        .padre = HMI_NODE_PARAMS,
        .hermano_anterior = HMI_NODE_SETPOINT,
        .hermano_siguiente = HMI_NODE_MODE,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_.configuracion.histeresis_deci_celsius,
        .valor_minimo = 1,
        .valor_maximo = 200,
        .paso = 1,
    },
    [HMI_NODE_MODE] = {
        .titulo = "Modo",
        .padre = HMI_NODE_PARAMS,
        .hermano_anterior = HMI_NODE_HYSTERESIS,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_.configuracion.modo_calentar,
        .valor_minimo = 0,
        .valor_maximo = 1,
        .paso = 1,
    },
};

static void hmi_escribir_linea_lcd(uint8_t fila, const char* texto)
{
    char linea[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    (void) snprintf(linea, sizeof(linea), "%-*.*s", HMI_CANTIDAD_COLUMNAS_LCD, HMI_CANTIDAD_COLUMNAS_LCD, texto);
    driver_lcd_set_position(1U, fila);
    driver_lcd_printf(linea);
}

static uint8_t hmi_obtener_primer_hermano(uint8_t nodo)
{
    uint8_t hermano = nodo;

    while (hmi_menu_tree_[hermano].hermano_anterior != 0U) {
        hermano = hmi_menu_tree_[hermano].hermano_anterior;
    }

    return hermano;
}

static uint8_t hmi_obtener_ultimo_hermano(uint8_t nodo)
{
    uint8_t hermano = nodo;

    while (hmi_menu_tree_[hermano].hermano_siguiente != 0U) {
        hermano = hmi_menu_tree_[hermano].hermano_siguiente;
    }

    return hermano;
}

static void hmi_formatear_linea_temperatura(char* linea, size_t tamano_linea, int16_t temperatura_deci_celsius)
{
    const bool es_negativa = (temperatura_deci_celsius < 0);
    const int16_t temperatura_absoluta = (int16_t) abs(temperatura_deci_celsius);
    const int16_t parte_entera = (int16_t) (temperatura_absoluta / 10);
    const int16_t parte_fraccionaria = (int16_t) (temperatura_absoluta % 10);

    if (es_negativa) {
        (void) snprintf(linea, tamano_linea, "-%d.%1dC", parte_entera, parte_fraccionaria);
    } else {
        (void) snprintf(linea, tamano_linea, "%d.%1dC", parte_entera, parte_fraccionaria);
    }
}

static void hmi_formatear_valor_deci(char* linea, size_t tamano_linea, int16_t valor_deci)
{
    const bool es_negativo = (valor_deci < 0);
    const int16_t valor_absoluto = (int16_t) abs(valor_deci);
    const int16_t parte_entera = (int16_t) (valor_absoluto / 10);
    const int16_t parte_fraccionaria = (int16_t) (valor_absoluto % 10);

    if (es_negativo) {
        (void) snprintf(linea, tamano_linea, "-%d.%1d", parte_entera, parte_fraccionaria);
    } else {
        (void) snprintf(linea, tamano_linea, "%d.%1d", parte_entera, parte_fraccionaria);
    }
}

static void hmi_formatear_modo_control(char* texto, size_t tamano_texto, int16_t valor_modo)
{
    if (valor_modo != 0) {
        (void) snprintf(texto, tamano_texto, "Calentar");
    } else {
        (void) snprintf(texto, tamano_texto, "Enfriar");
    }
}

static bool hmi_edicion_es_ciclica(uint8_t nodo)
{
    return (nodo == HMI_NODE_MODE);
}

static void hmi_dibujar_inicio(void)
{
    char linea_superior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char linea_inferior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char texto_temperatura[8];
    char texto_setpoint[8];
    char texto_histeresis[8];
    const char* texto_salida = hmi_.control.salida_activa ? "ON" : "OFF";
    const char* texto_modo = (hmi_.configuracion.modo_calentar != 0) ? "CAL" : "ENF";

    if (hmi_.control.sensor_resuelto && hmi_.sensor.temperatura_valida) {
        hmi_formatear_linea_temperatura(texto_temperatura, sizeof(texto_temperatura), hmi_.sensor.temperatura_deci_celsius);
    } else {
        (void) snprintf(texto_temperatura, sizeof(texto_temperatura), "--.-C");
    }

    hmi_formatear_valor_deci(texto_setpoint, sizeof(texto_setpoint), hmi_.configuracion.setpoint_deci_celsius);
    hmi_formatear_valor_deci(texto_histeresis, sizeof(texto_histeresis), hmi_.configuracion.histeresis_deci_celsius);

    (void) snprintf(linea_superior, sizeof(linea_superior), "T:%s %s %s", texto_temperatura, texto_salida, texto_modo);
    (void) snprintf(linea_inferior, sizeof(linea_inferior), "SP:%s H:%s", texto_setpoint, texto_histeresis);

    hmi_escribir_linea_lcd(1U, linea_superior);
    hmi_escribir_linea_lcd(2U, linea_inferior);
}

static void hmi_dibujar_menu(void)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_.interfaz.nodo_actual];
    const uint8_t nodo_padre = item_actual->padre;
    char linea_encabezado[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    if (nodo_padre == HMI_NODE_ROOT) {
        (void) snprintf(linea_encabezado, sizeof(linea_encabezado), "MENU");
    } else {
        (void) snprintf(linea_encabezado, sizeof(linea_encabezado), "MENU:%s", hmi_menu_tree_[nodo_padre].titulo);
    }

    hmi_escribir_linea_lcd(1U, linea_encabezado);
    hmi_escribir_linea_lcd(2U, item_actual->titulo);
}

static void hmi_dibujar_edicion(void)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_.interfaz.nodo_actual];
    char linea_valor[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    if (hmi_.interfaz.nodo_actual == HMI_NODE_MODE) {
        hmi_formatear_modo_control(linea_valor, sizeof(linea_valor), hmi_.interfaz.valor_edicion);
        hmi_escribir_linea_lcd(1U, item_actual->titulo);
        hmi_escribir_linea_lcd(2U, linea_valor);
        return;
    }

    if ((hmi_.interfaz.nodo_actual == HMI_NODE_SETPOINT) || (hmi_.interfaz.nodo_actual == HMI_NODE_HYSTERESIS)) {
        char texto_valor[8];

        hmi_formatear_valor_deci(texto_valor, sizeof(texto_valor), hmi_.interfaz.valor_edicion);
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%s", item_actual->titulo, texto_valor);
    } else {
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%d", item_actual->titulo, hmi_.interfaz.valor_edicion);
    }

    hmi_escribir_linea_lcd(1U, "EDITAR");
    hmi_escribir_linea_lcd(2U, linea_valor);
}

static void hmi_dibujar(void)
{
    if (!hmi_.interfaz.necesita_redibujado) {
        return;
    }

    switch (hmi_.interfaz.pantalla_actual) {
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

    hmi_.interfaz.necesita_redibujado = false;
}

static hmi_evento_t hmi_leer_evento(void)
{
    const uint8_t mascara_actual = button_read_all_pins();
    const uint8_t flancos_presionados =
        (uint8_t) (mascara_actual & (uint8_t) ~hmi_.interfaz.mascara_botones_anterior);

    hmi_.interfaz.mascara_botones_anterior = mascara_actual;

    if ((flancos_presionados & 0x01U) != 0U) {
        return HMI_EVENTO_MENU;
    }
    if ((flancos_presionados & 0x02U) != 0U) {
        return HMI_EVENTO_SUBIR;
    }
    if ((flancos_presionados & 0x04U) != 0U) {
        return HMI_EVENTO_BAJAR;
    }
    if ((flancos_presionados & 0x08U) != 0U) {
        return HMI_EVENTO_ACEPTAR;
    }

    return HMI_EVENTO_NINGUNO;
}

static void hmi_entrar_menu_raiz(void)
{
    hmi_.interfaz.nodo_actual = hmi_menu_tree_[HMI_NODE_ROOT].primer_hijo;
    hmi_.interfaz.pantalla_actual = HMI_PANTALLA_MENU;
    hmi_.interfaz.necesita_redibujado = true;
}

static void hmi_manejar_evento_menu(hmi_evento_t evento)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_.interfaz.nodo_actual];

    switch (evento) {
    case HMI_EVENTO_MENU:
        if (item_actual->padre == HMI_NODE_ROOT) {
            hmi_.interfaz.pantalla_actual = HMI_PANTALLA_INICIO;
        } else {
            hmi_.interfaz.nodo_actual = item_actual->padre;
        }
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
        if (item_actual->hermano_anterior != 0U) {
            hmi_.interfaz.nodo_actual = item_actual->hermano_anterior;
        } else {
            hmi_.interfaz.nodo_actual = hmi_obtener_ultimo_hermano(hmi_.interfaz.nodo_actual);
        }
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_BAJAR:
        if (item_actual->hermano_siguiente != 0U) {
            hmi_.interfaz.nodo_actual = item_actual->hermano_siguiente;
        } else {
            hmi_.interfaz.nodo_actual = hmi_obtener_primer_hermano(hmi_.interfaz.nodo_actual);
        }
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_ACEPTAR:
        if (item_actual->tipo == HMI_ITEM_MENU) {
            hmi_.interfaz.nodo_actual = item_actual->primer_hijo;
            hmi_.interfaz.pantalla_actual = HMI_PANTALLA_MENU;
        } else {
            hmi_.interfaz.valor_edicion = *item_actual->valor;
            hmi_.interfaz.pantalla_actual = HMI_PANTALLA_EDICION;
        }
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_NINGUNO:
    default:
        break;
    }
}

static void hmi_manejar_evento_edicion(hmi_evento_t evento)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_.interfaz.nodo_actual];
    const bool edicion_ciclica = hmi_edicion_es_ciclica(hmi_.interfaz.nodo_actual);
    int16_t nuevo_valor = hmi_.interfaz.valor_edicion;

    switch (evento) {
    case HMI_EVENTO_MENU:
        hmi_.interfaz.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
        nuevo_valor = (int16_t) (nuevo_valor + item_actual->paso);
        if (nuevo_valor > item_actual->valor_maximo) {
            nuevo_valor = edicion_ciclica ? item_actual->valor_minimo : item_actual->valor_maximo;
        }
        hmi_.interfaz.valor_edicion = nuevo_valor;
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_BAJAR:
        nuevo_valor = (int16_t) (nuevo_valor - item_actual->paso);
        if (nuevo_valor < item_actual->valor_minimo) {
            nuevo_valor = edicion_ciclica ? item_actual->valor_maximo : item_actual->valor_minimo;
        }
        hmi_.interfaz.valor_edicion = nuevo_valor;
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_ACEPTAR:
        *item_actual->valor = hmi_.interfaz.valor_edicion;
        hmi_.interfaz.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_.interfaz.necesita_redibujado = true;
        break;

    case HMI_EVENTO_NINGUNO:
    default:
        break;
    }
}

void hmi_init(void)
{
    hmi_.interfaz.pantalla_actual = HMI_PANTALLA_INICIO;
    hmi_.interfaz.nodo_actual = HMI_NODE_PARAMS;
    hmi_.interfaz.mascara_botones_anterior = 0U;
    hmi_.interfaz.valor_edicion = 0;
    hmi_.interfaz.necesita_redibujado = true;

    driver_lcd_write_char('\b');
    hmi_dibujar();
}

void hmi_process(void)
{
    const hmi_evento_t evento = hmi_leer_evento();

    switch (hmi_.interfaz.pantalla_actual) {
    case HMI_PANTALLA_INICIO:
        if (evento == HMI_EVENTO_MENU) {
            hmi_entrar_menu_raiz();
        }
        break;

    case HMI_PANTALLA_MENU:
        hmi_manejar_evento_menu(evento);
        break;

    case HMI_PANTALLA_EDICION:
        hmi_manejar_evento_edicion(evento);
        break;

    default:
        break;
    }

    hmi_dibujar();
}

void hmi_cargar_estado_sensor(bool temperatura_valida, int16_t temperatura_deci_celsius)
{
    const bool hubo_cambios = (hmi_.sensor.temperatura_valida != temperatura_valida)
        || (hmi_.sensor.temperatura_deci_celsius != temperatura_deci_celsius);

    hmi_.sensor.temperatura_valida = temperatura_valida;
    hmi_.sensor.temperatura_deci_celsius = temperatura_deci_celsius;

    if (hubo_cambios && (hmi_.interfaz.pantalla_actual == HMI_PANTALLA_INICIO)) {
        hmi_.interfaz.necesita_redibujado = true;
    }
}

int16_t hmi_obtener_setpoint_deci_celsius(void)
{
    return hmi_.configuracion.setpoint_deci_celsius;
}

uint16_t hmi_obtener_histeresis_deci_celsius(void)
{
    return (uint16_t) hmi_.configuracion.histeresis_deci_celsius;
}

bool hmi_modo_control_es_calentar(void)
{
    return (hmi_.configuracion.modo_calentar != 0);
}

void hmi_cargar_parametros_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   bool modo_calentar)
{
    hmi_.configuracion.setpoint_deci_celsius = setpoint_deci_celsius;
    hmi_.configuracion.histeresis_deci_celsius = (int16_t) histeresis_deci_celsius;
    hmi_.configuracion.modo_calentar = modo_calentar ? 1 : 0;
    hmi_.interfaz.valor_edicion = 0;
    hmi_.interfaz.necesita_redibujado = true;
}

void hmi_cargar_estado_control(bool salida_activa, bool sensor_resuelto, uint8_t indice_sensor_proceso)
{
    const bool hubo_cambios = (hmi_.control.salida_activa != salida_activa)
        || (hmi_.control.sensor_resuelto != sensor_resuelto);

    (void) indice_sensor_proceso;
    hmi_.control.salida_activa = salida_activa;
    hmi_.control.sensor_resuelto = sensor_resuelto;

    if (hubo_cambios && (hmi_.interfaz.pantalla_actual == HMI_PANTALLA_INICIO)) {
        hmi_.interfaz.necesita_redibujado = true;
    }
}
