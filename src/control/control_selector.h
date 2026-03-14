/**
 * @file control_selector.h
 * @brief Seleccion de estrategia de control en runtime sin memoria dinamica.
 */

#if !defined(CONTROL_CONTROL_SELECTOR_H_)
#define CONTROL_CONTROL_SELECTOR_H_

#include "control/control.h"
#include "control/control_on_off.h"

/**
 * @brief Tipos de control disponibles en tiempo de ejecucion.
 */
typedef enum {
    CONTROL_TIPO_NINGUNO = 0,
    CONTROL_TIPO_ON_OFF,
} control_tipo_t;

/**
 * @brief Contenedor de instancias concretas de control.
 *
 * Implementa la estrategia de instancias separadas: cada algoritmo tiene su
 * propia estructura estatica y el selector mantiene un puntero al control
 * activo. Esto evita `malloc` y facilita cambiar de estrategia en runtime.
 */
typedef struct {
    control_tipo_t tipo_activo;
    control_generico_t* control_activo;
    control_on_off_t instancia_on_off;
} control_selector_t;

/**
 * @brief Inicializa el selector sin ningun control activo.
 *
 * @param selector Selector a inicializar.
 *
 * @retval true Si la inicializacion fue correcta.
 * @retval false Si el puntero es invalido.
 */
bool control_selector_inicializar(control_selector_t* selector);

/**
 * @brief Deshabilita el control activo.
 *
 * @param selector Selector de estrategias.
 */
void control_selector_deshabilitar(control_selector_t* selector);

/**
 * @brief Activa la estrategia on/off sobre el selector.
 *
 * @param selector Selector de estrategias.
 * @param configuracion Configuracion inicial del control on/off.
 *
 * @retval true Si la estrategia pudo activarse.
 * @retval false Si algun puntero es invalido o la configuracion no es valida.
 */
bool control_selector_seleccionar_on_off(control_selector_t* selector,
                                         const control_on_off_configuracion_t* configuracion);

/**
 * @brief Obtiene el tipo de control actualmente activo.
 *
 * @param selector Selector de estrategias.
 *
 * @return Tipo activo. Si el selector no es valido, devuelve `CONTROL_TIPO_NINGUNO`.
 */
control_tipo_t control_selector_obtener_tipo(const control_selector_t* selector);

/**
 * @brief Obtiene una vista generica del control activo.
 *
 * @param selector Selector de estrategias.
 *
 * @return Puntero al control activo o `0` si no hay uno seleccionado.
 */
control_generico_t* control_selector_obtener_activo(const control_selector_t* selector);

/**
 * @brief Obtiene una vista generica constante del control activo.
 *
 * @param selector Selector de estrategias.
 *
 * @return Puntero constante al control activo o `0` si no hay uno seleccionado.
 */
const control_generico_t* control_selector_obtener_activo_const(const control_selector_t* selector);

#endif // CONTROL_CONTROL_SELECTOR_H_
