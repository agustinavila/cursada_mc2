set(CMAKE_TOOLCHAIN_FILE "./arm-none-eabi-gcc.cmake")
cmake_minimum_required(VERSION 3.10)

project(cursada-mc2)

set(C_FLAGS
    "-std=gnu11"
    "-mfpu=fpv4-sp-d16"
    "-mcpu=cortex-m4"
    "-mthumb -mabi=aapcs"
    "-mfloat-abi=hard -mfpu=fpv4-sp-d16"
    "-ffunction-sections -fdata-sections -fno-strict-aliasing"
    "-fno-builtin -fshort-enums"
    "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-main"
)
set(ASM_FLAGS "-g3 -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(LD_FLAGS
    "-mthumb"
    "-mcpu=cortex-m4"
    "-mfloat-abi=hard -mfpu=fpv4-sp-d16"
    "-Wl,--gc-sections"
    "--specs=nano.specs"
)

add_definitions(-D__REDLIB__ 
                        -DNDEBUG 
                        -D__CODE_RED 
                        -D__USE_LPCOPEN 
                        -DCORE_M4
                        -DNO_BOARD_LIB)

list(JOIN C_FLAGS " " C_FLAGS)
list(JOIN ASM_FLAGS " " ASM_FLAGS)
list(JOIN LD_FLAGS " " LD_FLAGS)

set(CMAKE_ASM_FLAGS_DEBUG "${ASM_FLAGS}")
set(CMAKE_ASM_FLAGS_RELEASE "${ASM_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -DDEBUG -g -Og")
set(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -DNDEBUG -O3 -g3 -flto")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LD_FLAGS} -g -Og")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LD_FLAGS} -O3 -g3 -flto")

set(LPCOPEN_CHIP_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_chip_43xx")
add_library(lpc_chip_43xx  ${LPCOPEN_CHIP_INCLUDE_PATHS})

# set(LPCOPEN_BOARD_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_board_nxp_lpcxpresso_4337")
# add_library(lpc_board_nxp_lpcxpresso_4337  ${LPCOPEN_BOARD_INCLUDE_PATHS})

add_subdirectory(${PROJECT_SOURCE_DIR}/src     build/src)

target_link_libraries(
  ${EXECUTABLE}
  lpc_chip_43xx
#   lpc_board_nxp_lpcxpresso_4337
  src
)

set_target_properties(lpc_chip_43xx PROPERTIES LINKER_LANGUAGE C)
# set_target_properties(lpc_board_nxp_lpcxpresso_4337 PROPERTIES LINKER_LANGUAGE C)