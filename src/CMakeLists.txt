add_library(cursada_mc2_startup OBJECT
    cr_startup_lpc43xx.c
    crp.c
    sysinit.c
)
target_link_libraries(cursada_mc2_startup PRIVATE lpc43xx_common)

add_subdirectory(Driver)

add_executable(cursada_mc2_app
    app/app.c
    control/control.c
    control/control_on_off.c
    control/control_selector.c
    hmi/hmi.c
    main.c
    $<TARGET_OBJECTS:cursada_mc2_startup>
)
target_include_directories(cursada_mc2_app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cursada_mc2_app PRIVATE
    cursada_mc2_drivers
    lpc_chip_43xx
)
lpc4337_configure_firmware_target(cursada_mc2_app)
