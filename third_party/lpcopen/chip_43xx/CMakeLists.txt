set(LPC43XX_CHIP_SOURCES
    # Clock tree, CGU/CCU y helpers base del chip.
    src/chip_18xx_43xx.c
    src/clock_18xx_43xx.c
    src/sysinit_18xx_43xx.c
    src/fpu_init.c

    # Perifericos efectivamente usados por el firmware actual.
    src/adc_18xx_43xx.c
    src/eeprom_18xx_43xx.c
    src/gpio_18xx_43xx.c
    src/ring_buffer.c
    src/ritimer_18xx_43xx.c
    src/stopwatch_18xx_43xx.c
    src/timer_18xx_43xx.c
    src/uart_18xx_43xx.c
)

add_library(lpc_chip_43xx STATIC ${LPC43XX_CHIP_SOURCES})
target_include_directories(lpc_chip_43xx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(lpc_chip_43xx PUBLIC lpc43xx_common)

# El vendor se compila sin warnings para no contaminar la salida del firmware.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lpc_chip_43xx PRIVATE -w)
endif()
