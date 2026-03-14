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

#define HMI_LCD_COLUMNS 16U
#define HMI_SENSOR_REFRESH_TICKS 50U
#define HMI_SENSOR_ROTATE_TICKS 150U
#define HMI_PROCESS_PERIOD_MS 20U

typedef enum {
    HMI_EVENT_NONE = 0,
    HMI_EVENT_MENU,
    HMI_EVENT_UP,
    HMI_EVENT_DOWN,
    HMI_EVENT_ENTER,
} hmi_event_t;

typedef enum {
    HMI_SCREEN_HOME = 0,
    HMI_SCREEN_MENU,
    HMI_SCREEN_EDIT,
} hmi_screen_t;

typedef enum {
    HMI_ITEM_MENU = 0,
    HMI_ITEM_INT_PARAM,
} hmi_item_kind_t;

typedef struct {
    const char* title;
    uint8_t parent;
    uint8_t first_child;
    uint8_t previous_sibling;
    uint8_t next_sibling;
    hmi_item_kind_t kind;
    int16_t* value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
} hmi_menu_item_t;

/**
 * @brief Arbol de menu codificado mediante indices de hijos y hermanos.
 *
 * Esta representacion evita memoria dinamica y mantiene la navegacion
 * deterministica dentro del microcontrolador.
 */
enum {
    HMI_NODE_ROOT = 0,
    HMI_NODE_PARAMS,
    HMI_NODE_MONITOR,
    HMI_NODE_SYSTEM,
    HMI_NODE_SETPOINT,
    HMI_NODE_TIME,
    HMI_NODE_ADC,
    HMI_NODE_BUTTON_MODE,
    HMI_NODE_BUZZER,
};

static int16_t hmi_setpoint_ = 25;
static int16_t hmi_time_seconds_ = 5;
static int16_t hmi_adc_preview_ = 0;
static int16_t hmi_button_mode_ = 1;
static int16_t hmi_buzzer_enable_ = 0;

static ds18b20_bus_driver_t hmi_temperature_bus_;
static uint8_t hmi_sensor_count_ = 0U;
static uint8_t hmi_sensor_display_index_ = 0U;
static bool hmi_sensor_temperature_valid_ = false;
static int16_t hmi_sensor_temperature_deci_celsius_ = 0;
static uint16_t hmi_sensor_refresh_ticks_ = 0U;
static uint16_t hmi_sensor_rotate_ticks_ = 0U;

static const onewire_pin_config_t hmi_ds18b20_pin_ = {
    .scu_port = 6U,
    .scu_pin = 1U,
    .scu_mode = (uint16_t) (MD_PUP | MD_EZI | MD_ZI),
    .scu_func = FUNC0,
    .gpio_port = 3U,
    .gpio_pin = 0U,
};

static const hmi_menu_item_t hmi_menu_tree_[] = {
    [HMI_NODE_ROOT] = {
        .title = "ROOT",
        .first_child = HMI_NODE_PARAMS,
        .kind = HMI_ITEM_MENU,
    },
    [HMI_NODE_PARAMS] = {
        .title = "Parametros",
        .parent = HMI_NODE_ROOT,
        .first_child = HMI_NODE_SETPOINT,
        .next_sibling = HMI_NODE_MONITOR,
        .kind = HMI_ITEM_MENU,
    },
    [HMI_NODE_MONITOR] = {
        .title = "Monitoreo",
        .parent = HMI_NODE_ROOT,
        .first_child = HMI_NODE_ADC,
        .previous_sibling = HMI_NODE_PARAMS,
        .next_sibling = HMI_NODE_SYSTEM,
        .kind = HMI_ITEM_MENU,
    },
    [HMI_NODE_SYSTEM] = {
        .title = "Sistema",
        .parent = HMI_NODE_ROOT,
        .first_child = HMI_NODE_BUZZER,
        .previous_sibling = HMI_NODE_MONITOR,
        .kind = HMI_ITEM_MENU,
    },
    [HMI_NODE_SETPOINT] = {
        .title = "Setpoint",
        .parent = HMI_NODE_PARAMS,
        .next_sibling = HMI_NODE_TIME,
        .kind = HMI_ITEM_INT_PARAM,
        .value = &hmi_setpoint_,
        .min_value = 0,
        .max_value = 100,
        .step = 1,
    },
    [HMI_NODE_TIME] = {
        .title = "Tiempo",
        .parent = HMI_NODE_PARAMS,
        .previous_sibling = HMI_NODE_SETPOINT,
        .kind = HMI_ITEM_INT_PARAM,
        .value = &hmi_time_seconds_,
        .min_value = 1,
        .max_value = 60,
        .step = 1,
    },
    [HMI_NODE_ADC] = {
        .title = "ADC preview",
        .parent = HMI_NODE_MONITOR,
        .next_sibling = HMI_NODE_BUTTON_MODE,
        .kind = HMI_ITEM_INT_PARAM,
        .value = &hmi_adc_preview_,
        .min_value = 0,
        .max_value = 1023,
        .step = 1,
    },
    [HMI_NODE_BUTTON_MODE] = {
        .title = "Btn mode",
        .parent = HMI_NODE_MONITOR,
        .previous_sibling = HMI_NODE_ADC,
        .kind = HMI_ITEM_INT_PARAM,
        .value = &hmi_button_mode_,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
    },
    [HMI_NODE_BUZZER] = {
        .title = "Buzzer",
        .parent = HMI_NODE_SYSTEM,
        .kind = HMI_ITEM_INT_PARAM,
        .value = &hmi_buzzer_enable_,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
    },
};

typedef struct {
    hmi_screen_t current_screen;
    uint8_t current_node;
    uint8_t last_button_mask;
    bool needs_redraw;
} hmi_state_t;

static hmi_state_t hmi_state_;

static void hmi_update_temperature(void)
{
    const uint8_t previous_sensor_count = hmi_sensor_count_;
    const uint8_t previous_display_index = hmi_sensor_display_index_;
    const bool previous_valid = hmi_sensor_temperature_valid_;
    const int16_t previous_temperature = hmi_sensor_temperature_deci_celsius_;
    int16_t raw_temperature = 0;

    ds18b20_bus_process(&hmi_temperature_bus_, HMI_PROCESS_PERIOD_MS);

    if (hmi_sensor_count_ == 0U) {
        hmi_sensor_temperature_valid_ = false;
    } else {
        if (hmi_sensor_display_index_ >= hmi_sensor_count_) {
            hmi_sensor_display_index_ = 0U;
        }

        if (ds18b20_bus_get_latest_raw(&hmi_temperature_bus_, hmi_sensor_display_index_, &raw_temperature)) {
            const int32_t scaled_value = (int32_t) raw_temperature * 10;
            if (scaled_value >= 0) {
                hmi_sensor_temperature_deci_celsius_ = (int16_t) ((scaled_value + 8) / 16);
            } else {
                hmi_sensor_temperature_deci_celsius_ = (int16_t) ((scaled_value - 8) / 16);
            }
            hmi_sensor_temperature_valid_ = true;
        } else {
            hmi_sensor_temperature_valid_ = false;
        }
    }

    if ((hmi_state_.current_screen == HMI_SCREEN_HOME)
        && ((previous_sensor_count != hmi_sensor_count_)
            || (previous_display_index != hmi_sensor_display_index_)
            || (previous_valid != hmi_sensor_temperature_valid_)
            || (previous_temperature != hmi_sensor_temperature_deci_celsius_))) {
        hmi_state_.needs_redraw = true;
    }
}

static void hmi_lcd_write_line(uint8_t row, const char* text)
{
    /**
     * @brief El LCD no borra automaticamente el resto de la linea.
     *
     * Por eso cada render escribe siempre 16 caracteres completos, rellenando
     * con espacios cuando hace falta.
     */
    char line[HMI_LCD_COLUMNS + 1U];
    size_t index = 0U;

    while ((index < HMI_LCD_COLUMNS) && (text[index] != '\0')) {
        line[index] = text[index];
        index++;
    }

    while (index < HMI_LCD_COLUMNS) {
        line[index] = ' ';
        index++;
    }

    line[HMI_LCD_COLUMNS] = '\0';
    driver_lcd_set_position(1U, row);
    driver_lcd_printf(line);
}

static uint8_t hmi_get_first_sibling(uint8_t node)
{
    uint8_t sibling = node;
    while (hmi_menu_tree_[sibling].previous_sibling != 0U) {
        sibling = hmi_menu_tree_[sibling].previous_sibling;
    }
    return sibling;
}

static uint8_t hmi_get_last_sibling(uint8_t node)
{
    uint8_t sibling = node;
    while (hmi_menu_tree_[sibling].next_sibling != 0U) {
        sibling = hmi_menu_tree_[sibling].next_sibling;
    }
    return sibling;
}

static void hmi_format_temperature_line(char* line, size_t line_size, int16_t deci_celsius)
{
    const bool is_negative = (deci_celsius < 0);
    const int16_t absolute_temperature = (int16_t) abs(deci_celsius);
    const int16_t integer_part = (int16_t) (absolute_temperature / 10);
    const int16_t fractional_part = (int16_t) (absolute_temperature % 10);

    if (is_negative) {
        (void) snprintf(line, line_size, "T:-%d.%1d C", integer_part, fractional_part);
    } else {
        (void) snprintf(line, line_size, "T: %d.%1d C", integer_part, fractional_part);
    }
}

static void hmi_render_home(void)
{
    char header_line[HMI_LCD_COLUMNS + 1U];
    char value_line[HMI_LCD_COLUMNS + 1U];

    if (hmi_sensor_count_ == 0U) {
        hmi_lcd_write_line(1U, "DS18B20");
        hmi_lcd_write_line(2U, "No detectado");
        return;
    }

    (void) snprintf(header_line,
                    sizeof(header_line),
                    "DS18B20 %u/%u",
                    (unsigned int) (hmi_sensor_display_index_ + 1U),
                    (unsigned int) hmi_sensor_count_);

    if (!hmi_sensor_temperature_valid_) {
        hmi_lcd_write_line(1U, header_line);
        hmi_lcd_write_line(2U, "Lectura fallida");
        return;
    }

    hmi_format_temperature_line(value_line,
                                sizeof(value_line),
                                hmi_sensor_temperature_deci_celsius_);

    hmi_lcd_write_line(1U, header_line);
    hmi_lcd_write_line(2U, value_line);
}

static void hmi_render_menu(void)
{
    const hmi_menu_item_t* current_item = &hmi_menu_tree_[hmi_state_.current_node];
    const uint8_t parent_node = current_item->parent;
    char header_line[HMI_LCD_COLUMNS + 1U];

    if (parent_node == HMI_NODE_ROOT) {
        (void) snprintf(header_line, sizeof(header_line), "MENU");
    } else {
        (void) snprintf(header_line, sizeof(header_line), "MENU:%s",
                        hmi_menu_tree_[parent_node].title);
    }

    hmi_lcd_write_line(1U, header_line);
    hmi_lcd_write_line(2U, current_item->title);
}

static void hmi_render_edit(void)
{
    const hmi_menu_item_t* current_item = &hmi_menu_tree_[hmi_state_.current_node];
    char value_line[HMI_LCD_COLUMNS + 1U];

    (void) snprintf(value_line, sizeof(value_line), "%s:%d",
                    current_item->title, *current_item->value);

    hmi_lcd_write_line(1U, "EDITAR");
    hmi_lcd_write_line(2U, value_line);
}

static void hmi_render(void)
{
    if (!hmi_state_.needs_redraw) {
        return;
    }

    switch (hmi_state_.current_screen) {
    case HMI_SCREEN_HOME:
        hmi_render_home();
        break;
    case HMI_SCREEN_MENU:
        hmi_render_menu();
        break;
    case HMI_SCREEN_EDIT:
        hmi_render_edit();
        break;
    default:
        break;
    }

    hmi_state_.needs_redraw = false;
}

static hmi_event_t hmi_poll_event(void)
{
    const uint8_t current_mask = button_read_all_pins();
    /**
     * @brief Las teclas se convierten en eventos por flanco.
     *
     * De esta manera una tecla mantenida no vuelve a dispararse continuamente
     * en cada iteracion del lazo principal.
     */
    const uint8_t pressed_edges = (uint8_t) (current_mask & (uint8_t) ~hmi_state_.last_button_mask);

    hmi_state_.last_button_mask = current_mask;

    if ((pressed_edges & 0x01U) != 0U) {
        return HMI_EVENT_MENU;
    }
    if ((pressed_edges & 0x02U) != 0U) {
        return HMI_EVENT_UP;
    }
    if ((pressed_edges & 0x04U) != 0U) {
        return HMI_EVENT_DOWN;
    }
    if ((pressed_edges & 0x08U) != 0U) {
        return HMI_EVENT_ENTER;
    }

    return HMI_EVENT_NONE;
}

static void hmi_enter_root_menu(void)
{
    hmi_state_.current_node = hmi_menu_tree_[HMI_NODE_ROOT].first_child;
    hmi_state_.current_screen = HMI_SCREEN_MENU;
    hmi_state_.needs_redraw = true;
}

static void hmi_handle_menu_event(hmi_event_t event)
{
    const hmi_menu_item_t* current_item = &hmi_menu_tree_[hmi_state_.current_node];

    switch (event) {
    case HMI_EVENT_MENU:
        if (current_item->parent == HMI_NODE_ROOT) {
            hmi_state_.current_screen = HMI_SCREEN_HOME;
        } else {
            hmi_state_.current_node = current_item->parent;
        }
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_UP:
        /** @brief La navegacion entre hermanos es ciclica. */
        if (current_item->previous_sibling != 0U) {
            hmi_state_.current_node = current_item->previous_sibling;
        } else {
            hmi_state_.current_node = hmi_get_last_sibling(hmi_state_.current_node);
        }
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_DOWN:
        /** @brief La navegacion entre hermanos es ciclica. */
        if (current_item->next_sibling != 0U) {
            hmi_state_.current_node = current_item->next_sibling;
        } else {
            hmi_state_.current_node = hmi_get_first_sibling(hmi_state_.current_node);
        }
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_ENTER:
        if (current_item->kind == HMI_ITEM_MENU) {
            hmi_state_.current_node = current_item->first_child;
            hmi_state_.current_screen = HMI_SCREEN_MENU;
        } else {
            hmi_state_.current_screen = HMI_SCREEN_EDIT;
        }
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_NONE:
    default:
        break;
    }
}

static void hmi_handle_edit_event(hmi_event_t event)
{
    const hmi_menu_item_t* current_item = &hmi_menu_tree_[hmi_state_.current_node];
    int16_t new_value = *current_item->value;

    switch (event) {
    case HMI_EVENT_MENU:
        hmi_state_.current_screen = HMI_SCREEN_MENU;
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_UP:
        new_value = (int16_t) (new_value + current_item->step);
        if (new_value > current_item->max_value) {
            new_value = current_item->max_value;
        }
        *current_item->value = new_value;
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_DOWN:
        new_value = (int16_t) (new_value - current_item->step);
        if (new_value < current_item->min_value) {
            new_value = current_item->min_value;
        }
        *current_item->value = new_value;
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_ENTER:
        hmi_state_.current_screen = HMI_SCREEN_MENU;
        hmi_state_.needs_redraw = true;
        break;

    case HMI_EVENT_NONE:
    default:
        break;
    }
}

void hmi_init(void)
{
    hmi_state_.current_screen = HMI_SCREEN_HOME;
    hmi_state_.current_node = HMI_NODE_PARAMS;
    hmi_state_.last_button_mask = 0U;
    hmi_state_.needs_redraw = true;

    driver_delay_init();
    driver_lcd_write_char('\b');
    if (ds18b20_bus_init(&hmi_temperature_bus_, &hmi_ds18b20_pin_)) {
        hmi_sensor_count_ = ds18b20_bus_discover(&hmi_temperature_bus_);
        if (hmi_sensor_count_ > 0U) {
            (void) ds18b20_bus_start_conversion(&hmi_temperature_bus_);
        }
    }
    hmi_update_temperature();
    hmi_render();
}

void hmi_process(void)
{
    const hmi_event_t event = hmi_poll_event();

    switch (hmi_state_.current_screen) {
    case HMI_SCREEN_HOME:
        if (event == HMI_EVENT_MENU) {
            hmi_enter_root_menu();
        }
        break;

    case HMI_SCREEN_MENU:
        hmi_handle_menu_event(event);
        break;

    case HMI_SCREEN_EDIT:
        hmi_handle_edit_event(event);
        break;

    default:
        break;
    }

    hmi_update_temperature();

    if (!ds18b20_bus_is_busy(&hmi_temperature_bus_)) {
        hmi_sensor_refresh_ticks_++;
        if (hmi_sensor_refresh_ticks_ >= HMI_SENSOR_REFRESH_TICKS) {
            hmi_sensor_refresh_ticks_ = 0U;
            if (hmi_sensor_count_ == 0U) {
                hmi_sensor_count_ = ds18b20_bus_discover(&hmi_temperature_bus_);
            }
            if (hmi_sensor_count_ > 0U) {
                (void) ds18b20_bus_start_conversion(&hmi_temperature_bus_);
            }
        }
    } else {
        hmi_sensor_refresh_ticks_ = 0U;
    }

    if ((hmi_state_.current_screen == HMI_SCREEN_HOME) && (hmi_sensor_count_ > 1U)) {
        hmi_sensor_rotate_ticks_++;
        if (hmi_sensor_rotate_ticks_ >= HMI_SENSOR_ROTATE_TICKS) {
            hmi_sensor_rotate_ticks_ = 0U;
            hmi_sensor_display_index_++;
            if (hmi_sensor_display_index_ >= hmi_sensor_count_) {
                hmi_sensor_display_index_ = 0U;
            }
            hmi_update_temperature();
        }
    } else {
        hmi_sensor_rotate_ticks_ = 0U;
    }

    hmi_render();
    /** @brief Antirrebote simple y control de ritmo del lazo de polling. */
    driver_delay_ms(HMI_PROCESS_PERIOD_MS);
}
