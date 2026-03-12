set(LPC43XX_CHIP_SOURCES
    src/adc_18xx_43xx.c
    src/aes_18xx_43xx.c
    src/atimer_18xx_43xx.c
    src/ccan_18xx_43xx.c
    src/chip_18xx_43xx.c
    src/clock_18xx_43xx.c
    src/dac_18xx_43xx.c
    src/eeprom_18xx_43xx.c
    src/emc_18xx_43xx.c
    src/enet_18xx_43xx.c
    src/evrt_18xx_43xx.c
    src/fpu_init.c
    src/gpdma_18xx_43xx.c
    src/gpiogroup_18xx_43xx.c
    src/gpio_18xx_43xx.c
    src/i2cm_18xx_43xx.c
    src/i2c_18xx_43xx.c
    src/i2s_18xx_43xx.c
    src/iap_18xx_43xx.c
    src/lcd_18xx_43xx.c
    src/otp_18xx_43xx.c
    src/pinint_18xx_43xx.c
    src/pmc_18xx_43xx.c
    src/ring_buffer.c
    src/ritimer_18xx_43xx.c
    src/rtc_18xx_43xx.c
    src/sct_18xx_43xx.c
    src/sct_pwm_18xx_43xx.c
    src/sdif_18xx_43xx.c
    src/sdio_18xx_43xx.c
    src/sdmmc_18xx_43xx.c
    src/spi_18xx_43xx.c
    src/ssp_18xx_43xx.c
    src/stopwatch_18xx_43xx.c
    src/sysinit_18xx_43xx.c
    src/timer_18xx_43xx.c
    src/uart_18xx_43xx.c
    src/wwdt_18xx_43xx.c
)

add_library(lpc_chip_43xx STATIC ${LPC43XX_CHIP_SOURCES})
target_include_directories(lpc_chip_43xx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(lpc_chip_43xx PUBLIC lpc43xx_common)
