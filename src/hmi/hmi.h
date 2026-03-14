/**
 * @file hmi.h
 * @brief Interfaz de la HMI jerárquica para LCD y pulsadores de la EDU-CIAA
 */

#if !defined(HMI_H_)
#define HMI_H_

/**
 * @brief Inicializa el estado interno de la HMI y dibuja la pantalla inicial.
 *
 * Debe llamarse una sola vez luego de haber inicializado los drivers de
 * hardware necesarios, en particular el LCD y la capa de delays.
 */
void hmi_init(void);

/**
 * @brief Procesa la navegación de la interfaz y actualiza el LCD si es necesario.
 *
 * Esta función debe llamarse de manera periódica desde el lazo principal.
 * Internamente:
 * - lee el estado de los pulsadores,
 * - detecta eventos de navegación,
 * - actualiza el estado de la HMI,
 * - y redibuja la pantalla cuando corresponde.
 */
void hmi_process(void);

#endif // HMI_H_
