/**
 * @file app.h
 * @brief Interfaz de la capa principal de aplicacion.
 */

#if !defined(APP_APP_H_)
#define APP_APP_H_

/**
 * @brief Inicializa los modulos de la aplicacion.
 *
 * Esta funcion concentra la inicializacion de drivers y servicios de nivel
 * aplicacion para mantener a `main.c` enfocado en el arranque de plataforma.
 */
void app_init(void);

/**
 * @brief Ejecuta una iteracion del lazo principal de la aplicacion.
 *
 * Debe llamarse de forma periodica desde el `while(1)` principal.
 */
void app_process(void);

#endif // APP_APP_H_
