/**
 * @file control.h
 * @brief Interfaz generica para estrategias de control.
 */

#if !defined(CONTROL_CONTROL_H_)
#define CONTROL_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct control_generico control_generico_t;

/**
 * @brief Tabla de operaciones de una estrategia de control concreta.
 *
 * Este patron permite emular polimorfismo en C: la aplicacion puede trabajar
 * contra `control_generico_t` sin conocer si la implementacion concreta es
 * on/off, PI, PID u otra.
 */
typedef struct {
    void (*reiniciar)(control_generico_t* control);
    bool (*procesar)(control_generico_t* control, int16_t medicion);
    bool (*salida_activa)(const control_generico_t* control);
    bool (*obtener_ultima_medicion)(const control_generico_t* control, int16_t* medicion);
} control_operaciones_t;

/**
 * @brief Estructura base comun a todas las estrategias de control.
 */
struct control_generico {
    const control_operaciones_t* operaciones;
};

/**
 * @brief Restablece el estado dinamico de un controlador.
 *
 * @param control Instancia base del controlador.
 */
void control_reiniciar(control_generico_t* control);

/**
 * @brief Procesa una nueva medicion con la estrategia configurada.
 *
 * @param control Instancia base del controlador.
 * @param medicion Medicion actual.
 *
 * @retval true Si la medicion fue procesada.
 * @retval false Si el controlador no es valido.
 */
bool control_procesar(control_generico_t* control, int16_t medicion);

/**
 * @brief Indica si la salida del controlador esta activa.
 *
 * @param control Instancia base del controlador.
 *
 * @retval true Si la salida esta activa.
 * @retval false Si la salida esta inactiva o el controlador no es valido.
 */
bool control_esta_salida_activa(const control_generico_t* control);

/**
 * @brief Obtiene la ultima medicion procesada por el controlador.
 *
 * @param control Instancia base del controlador.
 * @param medicion Variable de salida.
 *
 * @retval true Si hay una medicion valida.
 * @retval false Si no hay mediciones o el controlador no es valido.
 */
bool control_obtener_ultima_medicion(const control_generico_t* control, int16_t* medicion);

#endif // CONTROL_CONTROL_H_
