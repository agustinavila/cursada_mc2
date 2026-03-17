cmake_minimum_required(VERSION 3.21)

project(cursada_mc2 LANGUAGES C)

# Exporta compile_commands.json para IntelliSense, cppcheck y otras herramientas.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Carga los modulos auxiliares propios del proyecto.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(lpc4337)

# Rutas base del chip, scripts de linker y configuracion de OpenOCD.
set(LPC4337_CHIP_DIR "${CMAKE_SOURCE_DIR}/third_party/lpcopen/chip_43xx")
set(LPC4337_LINKER_DIR "${CMAKE_SOURCE_DIR}/platform/ldscripts/default")
set(OPENOCD_CONFIG "${CMAKE_SOURCE_DIR}/platform/openocd/ciaa-nxp.cfg" CACHE FILEPATH "OpenOCD configuration file for the EDU-CIAA-NXP debug probe")

# OpenOCD es opcional para compilar, pero necesario para los targets de flash.
find_program(OPENOCD_EXECUTABLE NAMES openocd)
if(NOT OPENOCD_EXECUTABLE)
    message(WARNING "OpenOCD was not found in PATH. Flash targets will fail until it is installed or added to PATH.")
endif()

# cppcheck es opcional. Si no esta disponible, el target de analisis se omite
# sin romper la configuracion ni la compilacion normal del proyecto.
find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CPPCHECK_EXECUTABLE)
    message(STATUS "Using cppcheck from: ${CPPCHECK_EXECUTABLE}")
else()
    message(STATUS "cppcheck was not found in PATH. The optional 'cppcheck' target will print a message and exit successfully.")
endif()

# Genera un linker script efectivo a partir de los fragmentos provistos por NXP.
lpc4337_generate_linker_script(LPC4337_LINKER_SCRIPT "${LPC4337_LINKER_DIR}")

# Biblioteca de configuracion comun para todos los targets del firmware.
# Agrupa solo configuracion compartida del micro y del chip vendor.
add_library(lpc43xx_common INTERFACE)
target_include_directories(lpc43xx_common SYSTEM INTERFACE
    "${LPC4337_CHIP_DIR}/inc"
)
target_compile_definitions(lpc43xx_common INTERFACE
    CORE_M4
    __USE_LPCOPEN
    NO_BOARD_LIB
    __NEWLIB__
)
target_compile_features(lpc43xx_common INTERFACE c_std_11)
target_compile_options(lpc43xx_common INTERFACE
    ${LPC4337_CPU_FLAGS}
    -ffreestanding
    -ffunction-sections
    -fdata-sections
    -fmessage-length=0
    -fno-common
    -fstack-usage
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<CONFIG:Release>:-O2>
)

# Primero se agrega el codigo vendor del chip y luego la aplicacion.
add_subdirectory(third_party/lpcopen/chip_43xx)
add_subdirectory(src)

# Target opcional de analisis estatico. No participa de la build normal y se
# puede invocar manualmente si la herramienta esta instalada.
if(CPPCHECK_EXECUTABLE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXECUTABLE}
            --template=gcc
            --enable=warning,style,performance,portability
            --error-exitcode=1
            --inline-suppr
            --suppress=missingIncludeSystem
            --suppress=constParameterPointer:third_party/lpcopen/chip_43xx/inc/*
            -D__GNUC__
            -DCORE_M4
            -Isrc
            -Isrc/drivers
            -Ithird_party/lpcopen/chip_43xx/inc
            src/main.c
            src/startup/sysinit.c
            src/hmi
            src/app
            src/control
            src/drivers
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        USES_TERMINAL
        VERBATIM
    )
else()
    add_custom_target(cppcheck
        COMMAND ${CMAKE_COMMAND} -E echo "cppcheck no esta disponible en PATH. Se omite el analisis."
        VERBATIM
    )
endif()
