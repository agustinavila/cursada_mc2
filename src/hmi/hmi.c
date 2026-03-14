/**
 * @file hmi.c
 * @brief Implementacion de la HMI jerarquica para LCD y pulsadores
 */

#include "hmi/hmi.h"

#include "Driver/buttons_driver.h"
#include "Driver/delay_driver.h"
#include "Driver/ds18b20_driver.h"
#include "Driver/lcd_driver.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HMI_CANTIDAD_COLUMNAS_LCD 16U
#define HMI_TICKS_ACTUALIZACION_SENSOR 50U
#define HMI_TICKS_ROTACION_SENSOR 150U
#define HMI_PERIODO_PROCESAMIENTO_MS 20U

typedef enum {
    HMI_EVENTO_NINGUNO = 0, /**< No se detecto ningun evento en el ciclo actual. */
    HMI_EVENTO_MENU,        /**< Se presiono la tecla de menu o cancelacion. */
    HMI_EVENTO_SUBIR,       /**< Se presiono la tecla de incremento o subida. */
    HMI_EVENTO_BAJAR,       /**< Se presiono la tecla de decremento o bajada. */
    HMI_EVENTO_ACEPTAR,     /**< Se presiono la tecla de ingreso o confirmacion. */
} hmi_evento_t;

typedef enum {
    HMI_PANTALLA_INICIO = 0, /**< Pantalla principal con el estado del sistema. */
    HMI_PANTALLA_MENU,       /**< Pantalla de navegacion por el arbol de menu. */
    HMI_PANTALLA_EDICION,    /**< Pantalla de edicion de un parametro puntual. */
} hmi_pantalla_t;

typedef enum {
    HMI_ITEM_MENU = 0,      /**< Nodo contenedor que agrupa hijos. */
    HMI_ITEM_PARAMETRO,     /**< Parametro entero editable desde la HMI. */
} hmi_tipo_item_t;

typedef struct {
    const char* titulo;          /**< Texto mostrado para este nodo. */
    uint8_t padre;               /**< Nodo padre dentro del arbol. */
    uint8_t primer_hijo;         /**< Primer hijo del nodo si es un menu. */
    uint8_t hermano_anterior;    /**< Hermano anterior en el mismo nivel. */
    uint8_t hermano_siguiente;   /**< Hermano siguiente en el mismo nivel. */
    hmi_tipo_item_t tipo;        /**< Tipo de nodo y comportamiento asociado. */
    int16_t* valor;              /**< Variable vinculada cuando el nodo es editable. */
    int16_t valor_minimo;        /**< Limite inferior permitido para la edicion. */
    int16_t valor_maximo;        /**< Limite superior permitido para la edicion. */
    int16_t paso;                /**< Incremento o decremento por pulsacion. */
} hmi_item_menu_t;

/**
 * @brief Arbol de menu codificado mediante indices de hijos y hermanos.
 *
 * Esta representacion evita memoria dinamica y mantiene la navegacion
 * deterministica dentro del microcontrolador.
 */
enum {
    HMI_NODE_ROOT = 0,   /**< Nodo raiz interno del arbol. */
    HMI_NODE_PARAMS,     /**< Menu de parametros configurables. */
    HMI_NODE_SETPOINT,   /**< Parametro de setpoint del control. */
    HMI_NODE_HYSTERESIS, /**< Parametro de banda de histeresis. */
    HMI_NODE_MODE,       /**< Parametro de modo calentar/enfriar. */
};

static int16_t hmi_setpoint_celsius_ = 27;
static int16_t hmi_histeresis_celsius_ = 2;
static int16_t hmi_modo_calentar_ = 1;

static ds18b20_bus_driver_t hmi_bus_temperatura_;
static uint8_t hmi_cantidad_sensores_ = 0U;
static uint8_t hmi_indice_sensor_mostrado_ = 0U;
static bool hmi_temperatura_sensor_valida_ = false;
static int16_t hmi_temperatura_sensor_deci_celsius_ = 0;
static uint16_t hmi_ticks_actualizacion_sensor_ = 0U;
static uint16_t hmi_ticks_rotacion_sensor_ = 0U;

static const onewire_pin_config_t hmi_pin_ds18b20_ = {
    .scu_port = 6U,
    .scu_pin = 1U,
    .scu_mode = (uint16_t) (MD_PUP | MD_EZI | MD_ZI),
    .scu_func = FUNC0,
    .gpio_port = 3U,
    .gpio_pin = 0U,
};

static const hmi_item_menu_t hmi_menu_tree_[] = {
    [HMI_NODE_ROOT] = {
        .titulo = "ROOT",
        .primer_hijo = HMI_NODE_PARAMS,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_PARAMS] = {
        .titulo = "Parametros",
        .padre = HMI_NODE_ROOT,
        .primer_hijo = HMI_NODE_SETPOINT,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_SETPOINT] = {
        .titulo = "Setpoint",
        .padre = HMI_NODE_PARAMS,
        .hermano_siguiente = HMI_NODE_HYSTERESIS,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_setpoint_celsius_,
        .valor_minimo = 0,
        .valor_maximo = 120,
        .paso = 1,
    },
    [HMI_NODE_HYSTERESIS] = {
        .titulo = "Histeresis",
        .padre = HMI_NODE_PARAMS,
        .hermano_anterior = HMI_NODE_SETPOINT,
        .hermano_siguiente = HMI_NODE_MODE,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_histeresis_celsius_,
        .valor_minimo = 1,
        .valor_maximo = 20,
        .paso = 1,
    },
    [HMI_NODE_MODE] = {
        .titulo = "Modo",
        .padre = HMI_NODE_PARAMS,
        .hermano_anterior = HMI_NODE_HYSTERESIS,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_modo_calentar_,
        .valor_minimo = 0,
        .valor_maximo = 1,
        .paso = 1,
    },
};

typedef struct {
    hmi_pantalla_t pantalla_actual;      /**< Pantalla actualmente visible. */
    uint8_t nodo_actual;                 /**< Nodo seleccionado en el arbol de menu. */
    uint8_t mascara_botones_anterior;    /**< Ultimo estado crudo de los pulsadores. */
    int16_t valor_edicion;               /**< Valor temporal mientras se edita un parametro. */
    bool necesita_redibujado;            /**< Indica si el LCD debe redibujarse. */
} hmi_estado_t;

static hmi_estado_t hmi_estado_;

static void hmi_actualizar_temperatura(void)
{
    const uint8_t cantidad_sensores_anterior = hmi_cantidad_sensores_;
    const uint8_t indice_mostrado_anterior = hmi_indice_sensor_mostrado_;
    const bool temperatura_valida_anterior = hmi_temperatura_sensor_valida_;
    const int16_t temperatura_anterior = hmi_temperatura_sensor_deci_celsius_;
    int16_t temperatura_cruda = 0;

    ds18b20_bus_process(&hmi_bus_temperatura_, HMI_PERIODO_PROCESAMIENTO_MS);

    if (hmi_cantidad_sensores_ == 0U) {
        hmi_temperatura_sensor_valida_ = false;
    } else {
        if (hmi_indice_sensor_mostrado_ >= hmi_cantidad_sensores_) {
            hmi_indice_sensor_mostrado_ = 0U;
        }

        if (ds18b20_bus_get_latest_raw(&hmi_bus_temperatura_, hmi_indice_sensor_mostrado_, &temperatura_cruda)) {
            const int32_t valor_escalado = (int32_t) temperatura_cruda * 10;
            if (valor_escalado >= 0) {
                hmi_temperatura_sensor_deci_celsius_ = (int16_t) ((valor_escalado + 8) / 16);
            } else {
                hmi_temperatura_sensor_deci_celsius_ = (int16_t) ((valor_escalado - 8) / 16);
            }
            hmi_temperatura_sensor_valida_ = true;
        } else {
            hmi_temperatura_sensor_valida_ = false;
        }
    }

    if ((hmi_estado_.pantalla_actual == HMI_PANTALLA_INICIO)
        && ((cantidad_sensores_anterior != hmi_cantidad_sensores_)
            || (indice_mostrado_anterior != hmi_indice_sensor_mostrado_)
            || (temperatura_valida_anterior != hmi_temperatura_sensor_valida_)
            || (temperatura_anterior != hmi_temperatura_sensor_deci_celsius_))) {
        hmi_estado_.necesita_redibujado = true;
    }
}

static void hmi_escribir_linea_lcd(uint8_t fila, const char* texto)
{
    /**
     * @brief El LCD no borra automaticamente el resto de la linea.
     *
     * Por eso cada render escribe siempre 16 caracteres completos, rellenando
     * con espacios cuando hace falta.
     */
    char line[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    (void) snprintf(line, sizeof(line), "%-*.*s", HMI_CANTIDAD_COLUMNAS_LCD, HMI_CANTIDAD_COLUMNAS_LCD, texto);
    driver_lcd_set_position(1U, fila);
    driver_lcd_printf(line);
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
        (void) snprintf(linea, tamano_linea, "T:-%d.%1d C", parte_entera, parte_fraccionaria);
    } else {
        (void) snprintf(linea, tamano_linea, "T: %d.%1d C", parte_entera, parte_fraccionaria);
    }
}

static const char* hmi_obtener_texto_modo_control_desde_valor(int16_t valor_modo)
{
    if (valor_modo != 0) {
        return "Calentar";
    }

    return "Enfriar";
}

static void hmi_dibujar_inicio(void)
{
    char linea_encabezado[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char linea_valor[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    if (hmi_cantidad_sensores_ == 0U) {
        hmi_escribir_linea_lcd(1U, "DS18B20");
        hmi_escribir_linea_lcd(2U, "No detectado");
        return;
    }

    (void) snprintf(linea_encabezado,
                    sizeof(linea_encabezado),
                    "DS18B20 %u/%u",
                    (unsigned int) (hmi_indice_sensor_mostrado_ + 1U),
                    (unsigned int) hmi_cantidad_sensores_);

    if (!hmi_temperatura_sensor_valida_) {
        hmi_escribir_linea_lcd(1U, linea_encabezado);
        hmi_escribir_linea_lcd(2U, "Lectura fallida");
        return;
    }

    hmi_formatear_linea_temperatura(linea_valor,
                                    sizeof(linea_valor),
                                    hmi_temperatura_sensor_deci_celsius_);

    hmi_escribir_linea_lcd(1U, linea_encabezado);
    hmi_escribir_linea_lcd(2U, linea_valor);
}

static void hmi_dibujar_menu(void)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_estado_.nodo_actual];
    const uint8_t nodo_padre = item_actual->padre;
    char linea_encabezado[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    if (nodo_padre == HMI_NODE_ROOT) {
        (void) snprintf(linea_encabezado, sizeof(linea_encabezado), "MENU");
    } else {
        (void) snprintf(linea_encabezado, sizeof(linea_encabezado), "MENU:%s",
                        hmi_menu_tree_[nodo_padre].titulo);
    }

    hmi_escribir_linea_lcd(1U, linea_encabezado);
    hmi_escribir_linea_lcd(2U, item_actual->titulo);
}

static void hmi_dibujar_edicion(void)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_estado_.nodo_actual];
    char linea_valor[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

    if (hmi_estado_.nodo_actual == HMI_NODE_MODE) {
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%s",
                        item_actual->titulo,
                        hmi_obtener_texto_modo_control_desde_valor(hmi_estado_.valor_edicion));
    } else {
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%d",
                        item_actual->titulo, hmi_estado_.valor_edicion);
    }

    hmi_escribir_linea_lcd(1U, "EDITAR");
    hmi_escribir_linea_lcd(2U, linea_valor);
}

static void hmi_dibujar(void)
{
    if (!hmi_estado_.necesita_redibujado) {
        return;
    }

    switch (hmi_estado_.pantalla_actual) {
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

    hmi_estado_.necesita_redibujado = false;
}

static hmi_evento_t hmi_leer_evento(void)
{
    const uint8_t mascara_actual = button_read_all_pins();
    /**
     * @brief Las teclas se convierten en eventos por flanco.
     *
     * De esta manera una tecla mantenida no vuelve a dispararse continuamente
     * en cada iteracion del lazo principal.
     */
    const uint8_t flancos_presionados =
        (uint8_t) (mascara_actual & (uint8_t) ~hmi_estado_.mascara_botones_anterior);

    hmi_estado_.mascara_botones_anterior = mascara_actual;

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
    hmi_estado_.nodo_actual = hmi_menu_tree_[HMI_NODE_ROOT].primer_hijo;
    hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
    hmi_estado_.necesita_redibujado = true;
}

static void hmi_manejar_evento_menu(hmi_evento_t evento)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_estado_.nodo_actual];

    switch (evento) {
    case HMI_EVENTO_MENU:
        if (item_actual->padre == HMI_NODE_ROOT) {
            hmi_estado_.pantalla_actual = HMI_PANTALLA_INICIO;
        } else {
            hmi_estado_.nodo_actual = item_actual->padre;
        }
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
        /** @brief La navegacion entre hermanos es ciclica. */
        if (item_actual->hermano_anterior != 0U) {
            hmi_estado_.nodo_actual = item_actual->hermano_anterior;
        } else {
            hmi_estado_.nodo_actual = hmi_obtener_ultimo_hermano(hmi_estado_.nodo_actual);
        }
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_BAJAR:
        /** @brief La navegacion entre hermanos es ciclica. */
        if (item_actual->hermano_siguiente != 0U) {
            hmi_estado_.nodo_actual = item_actual->hermano_siguiente;
        } else {
            hmi_estado_.nodo_actual = hmi_obtener_primer_hermano(hmi_estado_.nodo_actual);
        }
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_ACEPTAR:
        if (item_actual->tipo == HMI_ITEM_MENU) {
            hmi_estado_.nodo_actual = item_actual->primer_hijo;
            hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        } else {
            hmi_estado_.valor_edicion = *item_actual->valor;
            hmi_estado_.pantalla_actual = HMI_PANTALLA_EDICION;
        }
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_NINGUNO:
    default:
        break;
    }
}

static void hmi_manejar_evento_edicion(hmi_evento_t evento)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_estado_.nodo_actual];
    int16_t nuevo_valor = hmi_estado_.valor_edicion;

    switch (evento) {
    case HMI_EVENTO_MENU:
        hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
        nuevo_valor = (int16_t) (nuevo_valor + item_actual->paso);
        if (nuevo_valor > item_actual->valor_maximo) {
            nuevo_valor = item_actual->valor_maximo;
        }
        hmi_estado_.valor_edicion = nuevo_valor;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_BAJAR:
        nuevo_valor = (int16_t) (nuevo_valor - item_actual->paso);
        if (nuevo_valor < item_actual->valor_minimo) {
            nuevo_valor = item_actual->valor_minimo;
        }
        hmi_estado_.valor_edicion = nuevo_valor;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_ACEPTAR:
        *item_actual->valor = hmi_estado_.valor_edicion;
        hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_NINGUNO:
    default:
        break;
    }
}

void hmi_init(void)
{
    hmi_estado_.pantalla_actual = HMI_PANTALLA_INICIO;
    hmi_estado_.nodo_actual = HMI_NODE_PARAMS;
    hmi_estado_.mascara_botones_anterior = 0U;
    hmi_estado_.valor_edicion = 0;
    hmi_estado_.necesita_redibujado = true;

    driver_delay_init();
    driver_lcd_write_char('\b');
    if (ds18b20_bus_init(&hmi_bus_temperatura_, &hmi_pin_ds18b20_)) {
        hmi_cantidad_sensores_ = ds18b20_bus_discover(&hmi_bus_temperatura_);
        if (hmi_cantidad_sensores_ > 0U) {
            (void) ds18b20_bus_start_conversion(&hmi_bus_temperatura_);
        }
    }
    hmi_actualizar_temperatura();
    hmi_dibujar();
}

void hmi_process(void)
{
    const hmi_evento_t evento = hmi_leer_evento();

    switch (hmi_estado_.pantalla_actual) {
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

    hmi_actualizar_temperatura();

    if (!ds18b20_bus_is_busy(&hmi_bus_temperatura_)) {
        hmi_ticks_actualizacion_sensor_++;
        if (hmi_ticks_actualizacion_sensor_ >= HMI_TICKS_ACTUALIZACION_SENSOR) {
            hmi_ticks_actualizacion_sensor_ = 0U;
            if (hmi_cantidad_sensores_ == 0U) {
                hmi_cantidad_sensores_ = ds18b20_bus_discover(&hmi_bus_temperatura_);
            }
            if (hmi_cantidad_sensores_ > 0U) {
                (void) ds18b20_bus_start_conversion(&hmi_bus_temperatura_);
            }
        }
    } else {
        hmi_ticks_actualizacion_sensor_ = 0U;
    }

    if ((hmi_estado_.pantalla_actual == HMI_PANTALLA_INICIO) && (hmi_cantidad_sensores_ > 1U)) {
        hmi_ticks_rotacion_sensor_++;
        if (hmi_ticks_rotacion_sensor_ >= HMI_TICKS_ROTACION_SENSOR) {
            hmi_ticks_rotacion_sensor_ = 0U;
            hmi_indice_sensor_mostrado_++;
            if (hmi_indice_sensor_mostrado_ >= hmi_cantidad_sensores_) {
                hmi_indice_sensor_mostrado_ = 0U;
            }
            hmi_actualizar_temperatura();
        }
    } else {
        hmi_ticks_rotacion_sensor_ = 0U;
    }

    hmi_dibujar();
    /** @brief Antirrebote simple y control de ritmo del lazo de polling. */
    driver_delay_ms(HMI_PERIODO_PROCESAMIENTO_MS);
}

bool hmi_obtener_temperatura_sensor(uint8_t indice_sensor, int16_t* temperatura_deci_celsius)
{
    int16_t temperatura_cruda = 0;
    int32_t temperatura_escalada = 0;

    if (temperatura_deci_celsius == 0) {
        return false;
    }

    if (!ds18b20_bus_get_latest_raw(&hmi_bus_temperatura_, indice_sensor, &temperatura_cruda)) {
        return false;
    }

    temperatura_escalada = (int32_t) temperatura_cruda * 10;
    if (temperatura_escalada >= 0) {
        *temperatura_deci_celsius = (int16_t) ((temperatura_escalada + 8) / 16);
    } else {
        *temperatura_deci_celsius = (int16_t) ((temperatura_escalada - 8) / 16);
    }

    return true;
}

int16_t hmi_obtener_setpoint_deci_celsius(void)
{
    return (int16_t) (hmi_setpoint_celsius_ * 10);
}

uint16_t hmi_obtener_histeresis_deci_celsius(void)
{
    return (uint16_t) (hmi_histeresis_celsius_ * 10);
}

bool hmi_modo_control_es_calentar(void)
{
    return (hmi_modo_calentar_ != 0);
}
