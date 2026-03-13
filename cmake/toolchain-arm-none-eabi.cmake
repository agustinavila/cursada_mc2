set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(ARM_NONE_EABI_TOOLCHAIN_PATH "" CACHE PATH "Optional root or bin directory for the Arm GNU Toolchain")

set(_arm_gnu_hints)
if(ARM_NONE_EABI_TOOLCHAIN_PATH)
    list(APPEND _arm_gnu_hints "${ARM_NONE_EABI_TOOLCHAIN_PATH}")
    if(EXISTS "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin")
        list(APPEND _arm_gnu_hints "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin")
    endif()
endif()

find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_ASM_COMPILER NAMES arm-none-eabi-gcc HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_OBJDUMP NAMES arm-none-eabi-objdump HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_SIZE NAMES arm-none-eabi-size HINTS ${_arm_gnu_hints} REQUIRED)
find_program(CMAKE_GDB NAMES arm-none-eabi-gdb HINTS ${_arm_gnu_hints} REQUIRED)

get_filename_component(ARM_NONE_EABI_BIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
get_filename_component(ARM_NONE_EABI_ROOT "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

message(STATUS "Using Arm GNU Toolchain from: ${ARM_NONE_EABI_ROOT}")
