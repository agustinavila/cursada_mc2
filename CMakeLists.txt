cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "./arm-none-eabi-gcc.cmake")
set(CMAKE_VERBOSE_MAKEFILE ON)

project(cursada_mc2)

set(EXECUTABLE ${PROJECT_NAME}.axf)
set(LINKER_FILE ${CMAKE_SOURCE_DIR}/ld/cursada_Debug.ld)
set(LPCOPEN_CHIP_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_chip_43xx")
set(LPCOPEN_BOARD_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_board_nxp_lpcxpresso_4337")

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# List of source files
set(SRC_FILES src/main.c)

# Build the executable based on the source files
add_executable(${EXECUTABLE} ${SRC_FILES})

target_include_directories(${EXECUTABLE} PRIVATE
        inc
        ${LPCOPEN_CHIP_INCLUDE_PATHS}/inc
        )


# Compiler options
# target_compile_options(${EXECUTABLE} PRIVATE
#     -std=c99
#     -mcpu=cortex-m4
#     -mfpu=fpv4-sp-d16
#     -fno-common
#     -pedantic
#     -Wall
#     -Wconversion
#     -Wcast-align
#     -Wcast-qual
#     -Wlogical-op
#     -Wshadow
#     -Wsign-conversion
#     -Wfloat-equal
#     -c
#     -fmessage-length=0
#     -fno-builtin
#     -ffunction-sections
#     -fdata-sections
#     -fsingle-precision-constant
#     -v
#     -fmerge-constants
#     -mfloat-abi=softfp
#     -fstack-usage
#     -specs=redlib.specs
#     -mthumb
#     -Wall
#     -O0
#     -g3
#     )

set(C_FLAGS
-std=c99
-mcpu=cortex-m4
-mfpu=fpv4-sp-d16
-fno-common
-pedantic
-Wall
-Wconversion
-Wcast-align
-Wcast-qual
-Wlogical-op
-Wshadow
-Wsign-conversion
-Wfloat-equal
-c
-fmessage-length=0
-fno-builtin
-ffunction-sections
-fdata-sections
-fsingle-precision-constant
-v
-fmerge-constants
-mfloat-abi=softfp
-fstack-usage
-specs=redlib.specs
-mthumb
-Wall
-O0
-g3
    # "-std=gnu11"
    # "-mfpu=fpv4-sp-d16"
    # "-mcpu=cortex-m4"
    # "-mthumb -mabi=aapcs"
    # "-mfloat-abi=hard -mfpu=fpv4-sp-d16"
    # "-ffunction-sections -fdata-sections -fno-strict-aliasing"
    # "-fno-builtin -fshort-enums"
    # "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-main"
)

set(ASM_FLAGS "-DDEBUG 
-D__CODE_RED 
-DCORE_M4 
-D__USE_LPCOPEN 
-DNO_BOARD_LIB 
-D__LPC43XX__ 
-D__REDLIB__ 
-mcpu=cortex-m4 
-mfpu=fpv4-sp-d16 
-mfloat-abi=softfp 
-mthumb 
-D__REDLIB__ 
-specs=redlib.specs")

set(LD_FLAGS
    -nostdlib
    -L${LPCOPEN_CHIP_INCLUDE_PATHS}/Debug 
    -Xlinker -Map=${PROJECT_NAME}.map
    -mcpu=cortex-m4 
    -mfpu=fpv4-sp-d16 
    -mfloat-abi=softfp 
    -mthumb
    -T ${LINKER_FILE}
    -llpc_chip_43xx
    -Xlinker -print-memory-usage 
    -Xlinker --cref 
    -Xlinker --gc-sections
)

add_compile_definitions(-DDEBUG
        -D__CODE_RED
        -DCORE_M4
        -D__USE_LPCOPEN
        -DNO_BOARD_LIB
        -D__LPC43XX__
        -D__REDLIB__)

list(JOIN C_FLAGS " " C_FLAGS)
list(JOIN ASM_FLAGS " " ASM_FLAGS)
list(JOIN LD_FLAGS " " LD_FLAGS)

set(CMAKE_ASM_FLAGS_DEBUG "${ASM_FLAGS}")
set(CMAKE_ASM_FLAGS_RELEASE "${ASM_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -DDEBUG -g -Og")
set(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -DNDEBUG -O3 -g3 -flto")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LD_FLAGS} -g -Og")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LD_FLAGS} -O3 -g3 -flto")


# Optional: issue a message to be sure it uses the correct toolchain file.
message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")


add_library(lpc_chip_43xx  ${LPCOPEN_CHIP_INCLUDE_PATHS})
add_library(lpc_board_nxp_lpcxpresso_4337  ${LPCOPEN_BOARD_INCLUDE_PATHS})


set_target_properties(lpc_chip_43xx PROPERTIES LINKER_LANGUAGE C)
set_target_properties(lpc_board_nxp_lpcxpresso_4337 PROPERTIES LINKER_LANGUAGE C)

add_subdirectory(${PROJECT_SOURCE_DIR}/src     build/src)

# # Linker options
# target_link_options(${EXECUTABLE} PRIVATE
#         -nostdlib
#         -L${LPCOPEN_CHIP_INCLUDE_PATHS}/Debug 
#         -Xlinker -Map=${PROJECT_NAME}.map
#         -mcpu=cortex-m4 
#         -mfpu=fpv4-sp-d16 
#         -mfloat-abi=softfp 
#         -mthumb
#         -T ${LINKER_FILE}
#         -llpc_chip_43xx
#         -Xlinker -print-memory-usage 
#         -Xlinker --cref 
#         -Xlinker --gc-sections
#         )

target_link_libraries( ${EXECUTABLE}
    lpc_chip_43xx
    lpc_board_nxp_lpcxpresso_4337
    src
    )

# # Optional: Print executable size as part of the post build process
# add_custom_command(TARGET ${EXECUTABLE}
#         POST_BUILD
#         COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE})
