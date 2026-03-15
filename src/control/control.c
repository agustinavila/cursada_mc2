/**
 * @file control.c
 * @brief Implementacion de la interfaz generica de control.
 */

#include "control/control.h"

void control_reiniciar(control_generico_t* control)
{
    if ((control == 0) || (control->operaciones == 0) || (control->operaciones->reiniciar == 0)) {
        return;
    }

    control->operaciones->reiniciar(control);
}

bool control_procesar(control_generico_t* control, int16_t medicion)
{
    if ((control == 0) || (control->operaciones == 0) || (control->operaciones->procesar == 0)) {
        return false;
    }

    return control->operaciones->procesar(control, medicion);
}

bool control_esta_salida_activa(const control_generico_t* control)
{
    if ((control == 0) || (control->operaciones == 0) || (control->operaciones->salida_activa == 0)) {
        return false;
    }

    return control->operaciones->salida_activa(control);
}

bool control_obtener_ultima_medicion(const control_generico_t* control, int16_t* medicion)
{
    if ((control == 0) || (control->operaciones == 0) || (control->operaciones->obtener_ultima_medicion == 0)) {
        return false;
    }

    return control->operaciones->obtener_ultima_medicion(control, medicion);
}
