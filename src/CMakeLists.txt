set(CURSADA_MC2_STARTUP_SOURCES
    startup/cr_startup_lpc43xx.c
    startup/crp.c
    startup/sysinit.c
)

# Startup, vector table e inicializacion temprana del microcontrolador.
add_library(cursada_mc2_startup OBJECT
    ${CURSADA_MC2_STARTUP_SOURCES}
)
target_link_libraries(cursada_mc2_startup PRIVATE lpc43xx_common)
target_include_directories(cursada_mc2_startup PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/startup"
)

# Drivers y modulos de mas bajo nivel.
add_subdirectory(drivers)

set(CURSADA_MC2_APP_SOURCES
    app/app.c
    app/parametros.c
    app/parametros_default.c
    control/control_on_off.c
    hmi/hmi.c
    main.c
)

# Ejecutable principal del firmware con la logica de aplicacion.
add_executable(cursada_mc2_app
    ${CURSADA_MC2_APP_SOURCES}
    $<TARGET_OBJECTS:cursada_mc2_startup>
)

# Se agrega src como include raiz para poder incluir modulos por carpeta.
target_include_directories(cursada_mc2_app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cursada_mc2_app PRIVATE
    cursada_mc2_drivers
    lpc_chip_43xx
)

# Aplica linker script, flags de enlace y targets auxiliares .bin/.hex/flash.
lpc4337_configure_firmware_target(cursada_mc2_app)
