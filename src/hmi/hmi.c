/**
 * @file hmi.c
 * @brief Implementacion de la HMI jerarquica para LCD y pulsadores
 */

#include "hmi/hmi.h"

#include "Driver/buttons_driver.h"
#include "Driver/ds18b20_driver.h"
#include "Driver/lcd_driver.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HMI_CANTIDAD_COLUMNAS_LCD 16U
#define HMI_TICKS_ACTUALIZACION_SENSOR 50U
#define HMI_TICKS_RETORNO_RESUMEN 250U
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
    HMI_PANTALLA_CONFIRMACION, /**< Pantalla de confirmacion para acciones criticas. */
} hmi_pantalla_t;

typedef enum {
    HMI_ITEM_MENU = 0,      /**< Nodo contenedor que agrupa hijos. */
    HMI_ITEM_PARAMETRO,     /**< Parametro entero editable desde la HMI. */
    HMI_ITEM_ACCION,        /**< Accion disparada desde la HMI. */
} hmi_tipo_item_t;

typedef enum {
    HMI_VISTA_INICIO_RESUMEN = 0, /**< Vista principal con resumen del control. */
    HMI_VISTA_INICIO_SENSORES,    /**< Vista secundaria para recorrer sensores. */
} hmi_vista_inicio_t;

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
    HMI_NODE_SENSOR_CFG, /**< Menu de configuracion del sensor de proceso. */
    HMI_NODE_RESET,      /**< Accion para restablecer valores por defecto. */
    HMI_NODE_SETPOINT,   /**< Parametro de setpoint del control. */
    HMI_NODE_HYSTERESIS, /**< Parametro de banda de histeresis. */
    HMI_NODE_MODE,       /**< Parametro de modo calentar/enfriar. */
    HMI_NODE_SENSOR_MODE, /**< Parametro de modo auto/fijo del sensor de proceso. */
    HMI_NODE_PROCESS_SENSOR, /**< Parametro de seleccion del sensor de proceso. */
};

static int16_t hmi_setpoint_deci_celsius_ = 270;
static int16_t hmi_histeresis_deci_celsius_ = 20;
static int16_t hmi_modo_calentar_ = 1;
static int16_t hmi_sensor_proceso_automatico_ = 1;
static int16_t hmi_sensor_proceso_seleccion_ = 0;
static bool hmi_solicitud_restablecer_parametros_ = false;

static ds18b20_bus_driver_t hmi_bus_temperatura_;
static uint8_t hmi_cantidad_sensores_ = 0U;
static uint8_t hmi_indice_sensor_mostrado_ = 0U;
static uint8_t hmi_indice_sensor_proceso_ = 0U;
static bool hmi_temperatura_sensor_valida_ = false;
static int16_t hmi_temperatura_sensor_deci_celsius_ = 0;
static uint16_t hmi_ticks_actualizacion_sensor_ = 0U;
static uint16_t hmi_ticks_retorno_resumen_ = 0U;
static bool hmi_control_salida_activa_ = false;
static bool hmi_control_sensor_resuelto_ = false;

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
        .titulo = "Param control",
        .padre = HMI_NODE_ROOT,
        .primer_hijo = HMI_NODE_SETPOINT,
        .hermano_siguiente = HMI_NODE_SENSOR_CFG,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_SENSOR_CFG] = {
        .titulo = "Config sensor",
        .padre = HMI_NODE_ROOT,
        .primer_hijo = HMI_NODE_SENSOR_MODE,
        .hermano_anterior = HMI_NODE_PARAMS,
        .hermano_siguiente = HMI_NODE_RESET,
        .tipo = HMI_ITEM_MENU,
    },
    [HMI_NODE_RESET] = {
        .titulo = "Rest ajus def",
        .padre = HMI_NODE_ROOT,
        .hermano_anterior = HMI_NODE_SENSOR_CFG,
        .tipo = HMI_ITEM_ACCION,
    },
    [HMI_NODE_SETPOINT] = {
        .titulo = "Setpoint",
        .padre = HMI_NODE_PARAMS,
        .hermano_siguiente = HMI_NODE_HYSTERESIS,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_setpoint_deci_celsius_,
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
        .valor = &hmi_histeresis_deci_celsius_,
        .valor_minimo = 1,
        .valor_maximo = 200,
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
    [HMI_NODE_SENSOR_MODE] = {
        .titulo = "Modo",
        .padre = HMI_NODE_SENSOR_CFG,
        .hermano_siguiente = HMI_NODE_PROCESS_SENSOR,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_sensor_proceso_automatico_,
        .valor_minimo = 0,
        .valor_maximo = 1,
        .paso = 1,
    },
    [HMI_NODE_PROCESS_SENSOR] = {
        .titulo = "Sensor proc",
        .padre = HMI_NODE_SENSOR_CFG,
        .hermano_anterior = HMI_NODE_SENSOR_MODE,
        .tipo = HMI_ITEM_PARAMETRO,
        .valor = &hmi_sensor_proceso_seleccion_,
        .valor_minimo = 0,
        .valor_maximo = 0,
        .paso = 1,
    },
};

typedef struct {
    hmi_pantalla_t pantalla_actual;      /**< Pantalla actualmente visible. */
    hmi_vista_inicio_t vista_inicio;     /**< Subvista activa dentro de la pantalla principal. */
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

static void hmi_formatear_linea_temperatura_corta(char* linea, size_t tamano_linea, int16_t temperatura_deci_celsius)
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

static void hmi_formatear_linea_valor_deci(char* linea, size_t tamano_linea, int16_t valor_deci)
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

static void hmi_formatear_texto_modo_control(char* texto, size_t tamano_texto, int16_t valor_modo)
{
    if (valor_modo != 0) {
        (void) snprintf(texto, tamano_texto, "[X]CAL [ ]ENF");
    } else {
        (void) snprintf(texto, tamano_texto, "[ ]CAL [X]ENF");
    }
}

static void hmi_formatear_texto_modo_sensor(char* texto, size_t tamano_texto, int16_t valor_modo)
{
    if (valor_modo != 0) {
        (void) snprintf(texto, tamano_texto, "[X]Auto [ ]Fij");
    } else {
        (void) snprintf(texto, tamano_texto, "[ ]Auto [X]Fij");
    }
}

static void hmi_formatear_texto_sensor_proceso(char* texto, size_t tamano_texto, int16_t seleccion)
{
    uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE];

    if ((hmi_cantidad_sensores_ <= 1U) || (seleccion <= 0)) {
        (void) snprintf(texto, tamano_texto, "[X] Auto");
        return;
    }

    if (hmi_obtener_rom_sensor((uint8_t) (seleccion - 1), rom)) {
        /**
         * @brief El LCD solo permite 16 caracteres, por eso la ROM se muestra
         * abreviada usando los primeros 4 bytes en hexadecimal.
         */
        (void) snprintf(texto,
                        tamano_texto,
                        "[X]S%d %02X%02X%02X",
                        seleccion,
                        rom[0],
                        rom[1],
                        rom[2]);
        return;
    }

    (void) snprintf(texto, tamano_texto, "[X]S%d", seleccion);
}

static bool hmi_edicion_es_ciclica(uint8_t nodo)
{
    switch (nodo) {
    case HMI_NODE_MODE:
    case HMI_NODE_SENSOR_MODE:
    case HMI_NODE_PROCESS_SENSOR:
        return true;

    default:
        return false;
    }
}

static void hmi_entrar_vista_resumen(void)
{
    hmi_estado_.vista_inicio = HMI_VISTA_INICIO_RESUMEN;
    hmi_ticks_retorno_resumen_ = 0U;
    hmi_estado_.necesita_redibujado = true;
}

static void hmi_avanzar_vista_sensores(void)
{
    if (hmi_cantidad_sensores_ == 0U) {
        hmi_entrar_vista_resumen();
        return;
    }

    if (hmi_estado_.vista_inicio == HMI_VISTA_INICIO_RESUMEN) {
        hmi_estado_.vista_inicio = HMI_VISTA_INICIO_SENSORES;
        hmi_indice_sensor_mostrado_ = hmi_control_sensor_resuelto_ ? hmi_indice_sensor_proceso_ : 0U;
    } else {
        hmi_indice_sensor_mostrado_++;
        if (hmi_indice_sensor_mostrado_ >= hmi_cantidad_sensores_) {
            hmi_entrar_vista_resumen();
            return;
        }
    }

    hmi_ticks_retorno_resumen_ = 0U;
    hmi_actualizar_temperatura();
    hmi_estado_.necesita_redibujado = true;
}

static void hmi_dibujar_inicio_resumen(void)
{
    char linea_superior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char linea_inferior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char texto_temperatura[8];
    char texto_setpoint[8];
    char texto_histeresis[8];
    const char* texto_salida = hmi_control_salida_activa_ ? "ON" : "OFF";
    const char* texto_modo = (hmi_modo_calentar_ != 0) ? "CAL" : "ENF";

    if (hmi_control_sensor_resuelto_
        && hmi_obtener_temperatura_sensor(hmi_indice_sensor_proceso_, &hmi_temperatura_sensor_deci_celsius_)) {
        hmi_formatear_linea_temperatura_corta(texto_temperatura,
                                              sizeof(texto_temperatura),
                                              hmi_temperatura_sensor_deci_celsius_);
    } else {
        (void) snprintf(texto_temperatura, sizeof(texto_temperatura), "--.-C");
    }

    hmi_formatear_linea_valor_deci(texto_setpoint, sizeof(texto_setpoint), hmi_setpoint_deci_celsius_);
    hmi_formatear_linea_valor_deci(texto_histeresis, sizeof(texto_histeresis), hmi_histeresis_deci_celsius_);

    (void) snprintf(linea_superior,
                    sizeof(linea_superior),
                    "T:%s %s %s",
                    texto_temperatura,
                    texto_salida,
                    texto_modo);
    (void) snprintf(linea_inferior,
                    sizeof(linea_inferior),
                    "SP:%s H:%s",
                    texto_setpoint,
                    texto_histeresis);

    hmi_escribir_linea_lcd(1U, linea_superior);
    hmi_escribir_linea_lcd(2U, linea_inferior);
}

static void hmi_dibujar_inicio_sensor(void)
{
    char linea_superior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char linea_inferior[HMI_CANTIDAD_COLUMNAS_LCD + 1U];
    char texto_temperatura[8];

    if (hmi_cantidad_sensores_ == 0U) {
        hmi_escribir_linea_lcd(1U, "Sensores");
        hmi_escribir_linea_lcd(2U, "No detectado");
        return;
    }

    if (hmi_indice_sensor_mostrado_ >= hmi_cantidad_sensores_) {
        hmi_indice_sensor_mostrado_ = 0U;
    }

    if (!hmi_obtener_temperatura_sensor(hmi_indice_sensor_mostrado_, &hmi_temperatura_sensor_deci_celsius_)) {
        (void) snprintf(texto_temperatura, sizeof(texto_temperatura), "Lect fallida");
    } else {
        hmi_formatear_linea_temperatura_corta(texto_temperatura,
                                              sizeof(texto_temperatura),
                                              hmi_temperatura_sensor_deci_celsius_);
    }

    if (hmi_control_sensor_resuelto_ && (hmi_indice_sensor_mostrado_ == hmi_indice_sensor_proceso_)) {
        (void) snprintf(linea_superior, sizeof(linea_superior), "Sensor proc");
    } else {
        (void) snprintf(linea_superior,
                        sizeof(linea_superior),
                        "Sensor %u/%u",
                        (unsigned int) (hmi_indice_sensor_mostrado_ + 1U),
                        (unsigned int) hmi_cantidad_sensores_);
    }

    (void) snprintf(linea_inferior,
                    sizeof(linea_inferior),
                    "S%u %s",
                    (unsigned int) (hmi_indice_sensor_mostrado_ + 1U),
                    texto_temperatura);

    hmi_escribir_linea_lcd(1U, linea_superior);
    hmi_escribir_linea_lcd(2U, linea_inferior);
}

static void hmi_dibujar_inicio(void)
{
    if (hmi_estado_.vista_inicio == HMI_VISTA_INICIO_RESUMEN) {
        hmi_dibujar_inicio_resumen();
    } else {
        hmi_dibujar_inicio_sensor();
    }
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
        hmi_formatear_texto_modo_control(linea_valor, sizeof(linea_valor), hmi_estado_.valor_edicion);
        hmi_escribir_linea_lcd(1U, item_actual->titulo);
        hmi_escribir_linea_lcd(2U, linea_valor);
        return;
    } else if (hmi_estado_.nodo_actual == HMI_NODE_SENSOR_MODE) {
        hmi_formatear_texto_modo_sensor(linea_valor, sizeof(linea_valor), hmi_estado_.valor_edicion);
        hmi_escribir_linea_lcd(1U, item_actual->titulo);
        hmi_escribir_linea_lcd(2U, linea_valor);
        return;
    } else if (hmi_estado_.nodo_actual == HMI_NODE_PROCESS_SENSOR) {
        char texto_sensor[HMI_CANTIDAD_COLUMNAS_LCD + 1U];

        hmi_formatear_texto_sensor_proceso(texto_sensor, sizeof(texto_sensor), hmi_estado_.valor_edicion);
        hmi_escribir_linea_lcd(1U, item_actual->titulo);
        hmi_escribir_linea_lcd(2U, texto_sensor);
        return;
    } else if ((hmi_estado_.nodo_actual == HMI_NODE_SETPOINT)
               || (hmi_estado_.nodo_actual == HMI_NODE_HYSTERESIS)) {
        char texto_valor[8];

        hmi_formatear_linea_valor_deci(texto_valor, sizeof(texto_valor), hmi_estado_.valor_edicion);
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%s", item_actual->titulo, texto_valor);
    } else {
        (void) snprintf(linea_valor, sizeof(linea_valor), "%s:%d",
                        item_actual->titulo, hmi_estado_.valor_edicion);
    }

    hmi_escribir_linea_lcd(1U, "EDITAR");
    hmi_escribir_linea_lcd(2U, linea_valor);
}

static void hmi_dibujar_confirmacion(void)
{
    hmi_escribir_linea_lcd(1U, "Restablecer?");
    hmi_escribir_linea_lcd(2U, "ENT=SI MENU=NO");
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
    case HMI_PANTALLA_CONFIRMACION:
        hmi_dibujar_confirmacion();
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
        } else if (item_actual->tipo == HMI_ITEM_ACCION) {
            hmi_estado_.pantalla_actual = HMI_PANTALLA_CONFIRMACION;
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

static void hmi_manejar_evento_confirmacion(hmi_evento_t evento)
{
    switch (evento) {
    case HMI_EVENTO_MENU:
        hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_ACEPTAR:
        /* La accion real se delega a la app para mantener a la HMI desacoplada. */
        hmi_solicitud_restablecer_parametros_ = true;
        hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
    case HMI_EVENTO_BAJAR:
    case HMI_EVENTO_NINGUNO:
    default:
        break;
    }
}

static void hmi_manejar_evento_edicion(hmi_evento_t evento)
{
    const hmi_item_menu_t* item_actual = &hmi_menu_tree_[hmi_estado_.nodo_actual];
    const bool edicion_ciclica = hmi_edicion_es_ciclica(hmi_estado_.nodo_actual);
    const int16_t valor_minimo =
        (hmi_estado_.nodo_actual == HMI_NODE_PROCESS_SENSOR)
            ? ((hmi_cantidad_sensores_ > 1U) ? 1 : 0)
            : item_actual->valor_minimo;
    const int16_t valor_maximo =
        (hmi_estado_.nodo_actual == HMI_NODE_PROCESS_SENSOR)
            ? ((hmi_cantidad_sensores_ > 1U) ? (int16_t) hmi_cantidad_sensores_ : 0)
            : item_actual->valor_maximo;
    int16_t nuevo_valor = hmi_estado_.valor_edicion;

    switch (evento) {
    case HMI_EVENTO_MENU:
        hmi_estado_.pantalla_actual = HMI_PANTALLA_MENU;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_SUBIR:
        nuevo_valor = (int16_t) (nuevo_valor + item_actual->paso);
        if (nuevo_valor > valor_maximo) {
            nuevo_valor = edicion_ciclica ? valor_minimo : valor_maximo;
        }
        hmi_estado_.valor_edicion = nuevo_valor;
        hmi_estado_.necesita_redibujado = true;
        break;

    case HMI_EVENTO_BAJAR:
        nuevo_valor = (int16_t) (nuevo_valor - item_actual->paso);
        if (nuevo_valor < valor_minimo) {
            nuevo_valor = edicion_ciclica ? valor_maximo : valor_minimo;
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
    hmi_estado_.vista_inicio = HMI_VISTA_INICIO_RESUMEN;
    hmi_estado_.nodo_actual = HMI_NODE_PARAMS;
    hmi_estado_.mascara_botones_anterior = 0U;
    hmi_estado_.valor_edicion = 0;
    hmi_estado_.necesita_redibujado = true;

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
        } else if (evento == HMI_EVENTO_ACEPTAR) {
            hmi_avanzar_vista_sensores();
        }
        break;

    case HMI_PANTALLA_MENU:
        hmi_manejar_evento_menu(evento);
        break;

    case HMI_PANTALLA_EDICION:
        hmi_manejar_evento_edicion(evento);
        break;
    case HMI_PANTALLA_CONFIRMACION:
        hmi_manejar_evento_confirmacion(evento);
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

    if ((hmi_estado_.pantalla_actual == HMI_PANTALLA_INICIO)
        && (hmi_estado_.vista_inicio == HMI_VISTA_INICIO_SENSORES)) {
        hmi_ticks_retorno_resumen_++;
        if (hmi_ticks_retorno_resumen_ >= HMI_TICKS_RETORNO_RESUMEN) {
            hmi_entrar_vista_resumen();
        }
    }

    hmi_dibujar();
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

uint8_t hmi_obtener_cantidad_sensores(void)
{
    return hmi_cantidad_sensores_;
}

bool hmi_obtener_rom_sensor(uint8_t indice_sensor, uint8_t rom[PARAMETROS_SENSOR_ROM_SIZE])
{
    if (rom == 0) {
        return false;
    }

    return ds18b20_bus_get_rom_code(&hmi_bus_temperatura_, indice_sensor, rom);
}

int16_t hmi_obtener_setpoint_deci_celsius(void)
{
    return hmi_setpoint_deci_celsius_;
}

uint16_t hmi_obtener_histeresis_deci_celsius(void)
{
    return (uint16_t) hmi_histeresis_deci_celsius_;
}

bool hmi_modo_control_es_calentar(void)
{
    return (hmi_modo_calentar_ != 0);
}

bool hmi_sensor_proceso_es_automatico(void)
{
    return (hmi_sensor_proceso_automatico_ != 0);
}

uint8_t hmi_obtener_sensor_proceso_seleccion(void)
{
    if (hmi_sensor_proceso_seleccion_ <= 0) {
        return 1U;
    }

    return (uint8_t) hmi_sensor_proceso_seleccion_;
}

void hmi_cargar_parametros_control(int16_t setpoint_deci_celsius,
                                   uint16_t histeresis_deci_celsius,
                                   bool modo_calentar)
{
    hmi_setpoint_deci_celsius_ = setpoint_deci_celsius;
    hmi_histeresis_deci_celsius_ = (int16_t) histeresis_deci_celsius;
    hmi_modo_calentar_ = modo_calentar ? 1 : 0;
    hmi_estado_.valor_edicion = 0;
    hmi_estado_.necesita_redibujado = true;
}

void hmi_cargar_configuracion_sensor_proceso(bool automatico, uint8_t seleccion)
{
    hmi_sensor_proceso_automatico_ = automatico ? 1 : 0;
    if (hmi_cantidad_sensores_ > 1U) {
        hmi_sensor_proceso_seleccion_ = (seleccion > 0U) ? (int16_t) seleccion : 1;
    } else {
        hmi_sensor_proceso_seleccion_ = 0;
    }
    hmi_estado_.valor_edicion = 0;
    hmi_estado_.necesita_redibujado = true;
}

void hmi_cargar_estado_control(bool salida_activa,
                               bool sensor_resuelto,
                               uint8_t indice_sensor_proceso)
{
    const bool hubo_cambios = (hmi_control_salida_activa_ != salida_activa)
        || (hmi_control_sensor_resuelto_ != sensor_resuelto)
        || (hmi_indice_sensor_proceso_ != indice_sensor_proceso);

    hmi_control_salida_activa_ = salida_activa;
    hmi_control_sensor_resuelto_ = sensor_resuelto;
    hmi_indice_sensor_proceso_ = indice_sensor_proceso;

    if (hubo_cambios && (hmi_estado_.pantalla_actual == HMI_PANTALLA_INICIO)) {
        hmi_estado_.necesita_redibujado = true;
    }
}

bool hmi_consumir_solicitud_restablecer_parametros(void)
{
    const bool solicitud = hmi_solicitud_restablecer_parametros_;

    /* La solicitud se consume una sola vez para evitar repetir el reset. */
    hmi_solicitud_restablecer_parametros_ = false;
    return solicitud;
}
