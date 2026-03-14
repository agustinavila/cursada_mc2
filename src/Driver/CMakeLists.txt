set(CURSADA_MC2_DRIVER_SOURCES
    adc_driver.c
    buttons_driver.c
    buzzer_driver.c
    delay_driver.c
    ds18b20_driver.c
    keyboard_driver.c
    lcd_driver.c
    led_Driver.c
    onewire_driver.c
    timer_driver.c
    uart_driver.c
    uart_driver_irq.c
)

# Biblioteca estatica con todos los drivers propios del proyecto.
add_library(cursada_mc2_drivers STATIC
    ${CURSADA_MC2_DRIVER_SOURCES}
)

# Los headers de Driver quedan expuestos a los consumidores de esta biblioteca.
target_include_directories(cursada_mc2_drivers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Reutiliza la configuracion comun de flags, defines e includes del LPC4337.
target_link_libraries(cursada_mc2_drivers PUBLIC lpc43xx_common)
