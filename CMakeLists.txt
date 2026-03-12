cmake_minimum_required(VERSION 3.21)

project(cursada_mc2 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(lpc4337)

set(LPC4337_CHIP_DIR "${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc_chip_43xx")
set(LPC4337_LINKER_DIR "${CMAKE_SOURCE_DIR}/platform/lpc43xx/ldscripts/default")
set(OPENOCD_CONFIG "${CMAKE_SOURCE_DIR}/platform/openocd/ciaa-nxp.cfg" CACHE FILEPATH "OpenOCD configuration file for the EDU-CIAA-NXP debug probe")

find_program(OPENOCD_EXECUTABLE NAMES openocd)
if(NOT OPENOCD_EXECUTABLE)
    message(WARNING "OpenOCD was not found in PATH. Flash targets will fail until it is installed or added to PATH.")
endif()

lpc4337_generate_linker_script(LPC4337_LINKER_SCRIPT "${LPC4337_LINKER_DIR}")

add_library(lpc43xx_common INTERFACE)
target_include_directories(lpc43xx_common INTERFACE
    "${LPC4337_CHIP_DIR}/inc"
    "${CMAKE_SOURCE_DIR}/src"
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

add_subdirectory(platform/lpc43xx/lpc_chip_43xx)
add_subdirectory(src)
