add_library(cursada_mc2_drivers STATIC
    adc_driver.c
    buttons_driver.c
    buzzer_driver.c
    keyboard_driver.c
    lcd_driver.c
    led_Driver.c
    timer_driver.c
    uart_driver.c
    uart_driver_irq.c
)
target_include_directories(cursada_mc2_drivers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cursada_mc2_drivers PUBLIC lpc43xx_common)
