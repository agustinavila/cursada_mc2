add_library(cursada_mc2_test_support STATIC
    support/test_harness.c
)
target_include_directories(cursada_mc2_test_support PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/support"
)
target_link_libraries(cursada_mc2_test_support PUBLIC cursada_mc2_host_common)

add_library(cursada_mc2_control_host STATIC
    "${CMAKE_SOURCE_DIR}/src/control/control.c"
    "${CMAKE_SOURCE_DIR}/src/control/control_on_off.c"
    "${CMAKE_SOURCE_DIR}/src/control/control_selector.c"
)
target_include_directories(cursada_mc2_control_host PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(cursada_mc2_control_host PUBLIC cursada_mc2_host_common)

add_library(cursada_mc2_parametros_host STATIC
    "${CMAKE_SOURCE_DIR}/src/app/parametros.c"
    "${CMAKE_SOURCE_DIR}/src/app/parametros_default.c"
    support/fake_eeprom_driver.c
)
target_include_directories(cursada_mc2_parametros_host PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/support"
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(cursada_mc2_parametros_host PUBLIC cursada_mc2_host_common)

add_executable(control_tests
    control/control_tests.c
)
target_link_libraries(control_tests PRIVATE
    cursada_mc2_control_host
    cursada_mc2_test_support
)
add_test(NAME control_tests COMMAND control_tests)

add_executable(parametros_tests
    app/parametros_tests.c
)
target_link_libraries(parametros_tests PRIVATE
    cursada_mc2_parametros_host
    cursada_mc2_test_support
)
add_test(NAME parametros_tests COMMAND parametros_tests)

if(CURSADA_MC2_ENABLE_COVERAGE)
    find_program(HOST_GCOV_EXECUTABLE NAMES gcov REQUIRED)

    add_custom_target(coverage
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
        COMMAND "${CMAKE_COMMAND}"
            -DGCOV_EXECUTABLE=${HOST_GCOV_EXECUTABLE}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/RunGcov.cmake"
        DEPENDS control_tests parametros_tests
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        USES_TERMINAL
        VERBATIM
    )
endif()
