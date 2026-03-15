set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Permite forzar una instalacion concreta del toolchain sin hardcodear rutas.
set(ARM_NONE_EABI_TOOLCHAIN_PATH "" CACHE PATH "Optional root or bin directory for the Arm GNU Toolchain")

# Construye una lista de rutas candidatas para buscar las herramientas.
set(_arm_gnu_hints)
if(ARM_NONE_EABI_TOOLCHAIN_PATH)
    list(APPEND _arm_gnu_hints "${ARM_NONE_EABI_TOOLCHAIN_PATH}")
    if(EXISTS "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin")
        list(APPEND _arm_gnu_hints "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin")
    endif()
endif()

# Herramientas requeridas para compilar, enlazar y generar artefactos.
find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_ASM_COMPILER NAMES arm-none-eabi-gcc HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_OBJDUMP NAMES arm-none-eabi-objdump HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_SIZE NAMES arm-none-eabi-size HINTS ${_arm_gnu_hints} REQUIRED)

# GDB se usa para debug, pero no es obligatorio para compilar en CI o local.
find_program(CMAKE_GDB NAMES arm-none-eabi-gdb HINTS ${_arm_gnu_hints})

# Se conservan rutas derivadas del compilador para mensajes y tooling auxiliar.
get_filename_component(ARM_NONE_EABI_BIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
get_filename_component(ARM_NONE_EABI_ROOT "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)

# No se usa sysroot; las herramientas se toman del PATH o de la ruta provista.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

message(STATUS "Using Arm GNU Toolchain from: ${ARM_NONE_EABI_ROOT}")
if(NOT CMAKE_GDB)
    message(STATUS "arm-none-eabi-gdb was not found. Build targets will work, but debug integrations that rely on CMAKE_GDB may be unavailable.")
endif()
