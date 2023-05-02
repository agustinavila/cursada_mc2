cmake_minimum_required(VERSION 3.1)

if(CMAKE_HOST_WIN32)
  set(EXECUTABLE_SUFFIX .exe)
endif()

# Supported C libraries
set(SUPPORTED_CLIBS newlib newlib-nano redlib)

# Supported hosting settings
set(SUPPORTED_HOSTING none nohost semihosting)

# Supported target devices
set(SUPPORTED_DEVICES LPC4337)

# Check if C++ is enabled
if(CPP)
  set(LANG "CXX")
else()
  set(LANG "C")
endif()
message(STATUS "Target language: " ${LANG})

# Set toolchain file if not specified
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake)
else()
  if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
  endif()
endif()
message(STATUS "Toolchain file: " ${CMAKE_TOOLCHAIN_FILE})

project(cursada_mc2)

# Supported build configurations: "Release" and "Debug"
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
  "Reset the configurations to what we need"
  FORCE
)

# Set default output name if not specified
if (NOT OUTPUT_NAME)
  set(OUTPUT_NAME ${CMAKE_PROJECT_NAME})
endif()
message(STATUS "Output name: " ${OUTPUT_NAME})

# Set default build type if not specified
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
else()
  if(NOT ";${CMAKE_CONFIGURATION_TYPES};" MATCHES ${CMAKE_BUILD_TYPE})
    message(FATAL_ERROR "Specified build type is not supported: " ${CMAKE_BUILD_TYPE})
  endif()
endif()
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# Set C/C++ library if not specified
if(NOT DEFINED CLIB)
  set(CLIB redlib)
else()
    # Check if library is supported
    if(NOT ";${SUPPORTED_CLIBS};" MATCHES ${CLIB})
      message(FATAL_ERROR "Specified library is not supported: " ${CLIB})
    endif()
    # Check if language/library combination is supported
    if(${CPP} AND ${CLIB} STREQUAL redlib)
      message(FATAL_ERROR "Specified library does not support C++: " ${CLIB})
    endif()
endif()
message(STATUS "C/C++ library: " ${CLIB})

# Check if Code Read Protection is enabled
if(CRP)
  message(STATUS "Code Read Protection: Enabled")
else()
  message(STATUS "Code Read Protection: Disabled")
endif()

# Set hosting if not specified
if(NOT DEFINED HOSTING)
  set(HOSTING nohost)
else()
    # Check if C library is supported
    if(NOT ";${SUPPORTED_HOSTING};" STREQUAL ${HOSTING})
      message(FATAL_ERROR "Specified hosting setting is not supported: " ${HOSTING})
    endif()
endif()
message(STATUS "Hosting setting: " ${HOSTING})

# Set hosting if not specified
if(NOT DEFINED DEVICE)
  set(DEVICE LPC4337)
else()
    # Check if device is supported
    if(NOT ";${SUPPORTED_DEVICES};" STREQUAL ${DEVICE})
      message(FATAL_ERROR "Specified device is not supported: " ${DEVICE})
    endif()
endif()
message(STATUS "Target device: " ${DEVICE})

# Set flash driver file if not specified
if(NOT DEFINED FLASHDRIVER)
  set(FLASHDRIVER LPC18x7_43x7_2x512_BootA.cfx)
endif()
message(STATUS "MCUXpresso flash driver file: " ${FLASHDRIVER})

# Set reset script if not specified
if(NOT DEFINED RESETSCRIPT)
  set(RESETSCRIPT LPC18LPC43InternalFLASHBootResetscript.scp)
endif()
message(STATUS "MCUXpresso reset script: " ${RESETSCRIPT})

# Check if MCUXpresso checksum tool is present
if(EXISTS ${TOOLCHAIN_PREFIX}/../binaries/checksum${EXECUTABLE_SUFFIX})
  set(CHECKSUM_TOOL ${TOOLCHAIN_PREFIX}/../binaries/checksum)
  set(HAVE_CHECKSUM_TOOL ON)
  message(STATUS "MCUXpresso checksum tool found: " ${CHECKSUM_TOOL})
  message(STATUS "'make bin' target will add checksum information.")
else()
  set(HAVE_CHECKSUM_TOOL OFF)
  message(STATUS "WARNING: MCUXpresso checksum tool not found. 'make bin' target will NOT add checksum information.")
endif()

# Check if specified path to the OpenOCD binary is correct
if(EXISTS ${OPENOCD_BINARY})
  if(${HAVE_CHECKSUM_TOOL})
    # Configure OpenOCD flashing depending on build settings
    if(FLASHDRIVER STREQUAL LPC18x7_43x7_2x512_BootA.cfx)
      set(FLASH_BANK 0)
      set(FLASH_BANK_ADDRESS 0x1a000000)
    elseif(FLASHDRIVER STREQUAL LPC18x7_43x7_2x512_BootB.cfx)
      set(FLASH_BANK 1)
      set(FLASH_BANK_ADDRESS 0x1b000000)
    else()
      message(STATUS "WARNING: OpenOCD does not support this flash driver: " ${FLASHDRIVER})
      message(STATUS "'make flash' and 'make erase' targets will NOT be available.")
    endif()
    set(HAVE_OPENOCD ON)
    message(STATUS "OpenOCD found: " ${OPENOCD_BINARY})
    message(STATUS "'make flash' and 'make erase' targets will be available.")
    # Set OpenOCD configuration file if not specified
    if(NOT DEFINED OPENOCD_CONFIG)
      set(OPENOCD_CONFIG stlink-v2_lpc43xx.cfg)
    endif()
    message(STATUS "OpenOCD configuration: " ${OPENOCD_CONFIG})
    # Set OpenOCD transport if not specified
    if(NOT DEFINED OPENOCD_TRANSPORT)
      set(OPENOCD_TRANSPORT hla_jtag)
    endif()
    message(STATUS "OpenOCD transport: " ${OPENOCD_TRANSPORT})
  else()
    set(HAVE_OPENOCD OFF)
    message(STATUS "WARNING: MCUXpresso checksum tool not found. 'make flash' and 'make erase' targets will NOT be available.")
  endif()
else()
  set(HAVE_OPENOCD OFF)
  message(STATUS "WARNING: OpenOCD path not specified. 'make flash' and 'make erase' targets will NOT be available.")
endif()

# Store paths to commonly used files/locations for convenience
set(APP_PATH ${CMAKE_SOURCE_DIR}/sources/application)
set(PLATFORM_FILE ${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx.cmake)
set(PLATFORM_TARGETS ${CMAKE_SOURCE_DIR}/platform/lpc43xx/lpc43xx_targets.cmake)
set(SCRIPTS_DIR ${PROJECT_SOURCE_DIR}/scripts)

# Set Linker Script dir if not set
if(NOT DEFINED LINKER_SCRIPT_DIR)
  set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/platform/lpc43xx/ldscripts/default")
else()
  set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT_DIR}")
endif()
message(STATUS "Linker script directory: " ${LINKER_SCRIPT_DIR})

#########################
# LPCOpen: Chip Library #
#########################
if(NOT DEFINED LPCOPEN_VERSION)
  set(LPCOPEN_VERSION 3.02)
endif()
set(LPCOPEN_PATH ${CMAKE_SOURCE_DIR}/../MCUXpressoIDE_11.7.0_9198/workspace)
# Chip Library
if(EXISTS ${LPCOPEN_PATH}/lpc_chip_43xx)
  set(LPCOPEN_CHIP_PATH ${LPCOPEN_PATH}/lpc_chip_43xx)
  add_subdirectory(${LPCOPEN_CHIP_PATH} "${CMAKE_CURRENT_BINARY_DIR}/lpc_chip_43xx")
  message(STATUS "LPCOpen chip library (lpc_chip_43xx) path: " ${LPCOPEN_CHIP_PATH})
else()
  message(FATAL_ERROR "LPCOpen chip library (lpc_chip_43xx) source folder not found in: ${LPCOPEN_PATH}/lpc_chip_43xx")
endif()

#########################
# Board Support Package #
#########################
if(NOT DEFINED BSP_NAME AND NOT DEFINED BSP_VERSION)
  message(STATUS "No BSP specified. BSP will NOT be included in build.")
  add_definitions(-DNO_BOARD_LIB)
else()
  if(NOT DEFINED BSP_NAME)
    message(STATUS "WARNING: No BSP Name specified.")
  endif()
  if(NOT DEFINED BSP_VERSION)
    message(STATUS "WARNING: No BSP Version specified.")
  endif()
  set(BSP_PATH ${CMAKE_SOURCE_DIR}/sources/bsp/${BSP_NAME}/${BSP_VERSION})
  # Check if path exists
  if(NOT EXISTS ${BSP_PATH})
    message(FATAL_ERROR "BSP path is invalid: " ${BSP_PATH})
  endif()
  add_subdirectory(${BSP_PATH})
  message(STATUS "BSP Name: " ${BSP_NAME})
  message(STATUS "BSP Version: " ${BSP_VERSION})
endif()

###############
# Application #
###############
add_subdirectory(src)

# Print linker script used
message(STATUS "Linker script filename: " ${LINKER_SCRIPT})
message(STATUS "Linker script component (LIB): " ${LINKER_SCRIPT_LIB})
if(NOT ${LINKER_SCRIPT_LIB_CPP} STREQUAL "")
  message(STATUS "Linker script component (LIB CPP): " ${LINKER_SCRIPT_LIB_CPP})
endif()
message(STATUS "Linker script component (MEM): " ${LINKER_SCRIPT_MEM})
message(STATUS "Linker script component (SECTIONS): " ${LINKER_SCRIPT_SECTIONS})

# Check whether *printf float and char printf arguments are applicable
if(${PRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano OR NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: printf float argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${PRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: printf float argument not valid for library: " ${CLIB})
  endif()
endif()
if(${SPRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: sprintf float argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${SPRINTF_FLOAT})
  if(NOT ${CLIB} STREQUAL newlib-nano)
    message(STATUS "WARNING: sprintf float argument not valid for library: " ${CLIB})
  endif()
endif()
if(${CHAR_PRINTF})
  if(NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: char printf argument not valid for library: " ${CLIB})
  endif()
elseif(NOT ${CHAR_PRINTF})
  if(NOT ${CLIB} STREQUAL redlib)
    message(STATUS "WARNING: char printf argument not valid for library: " ${CLIB})
  endif()
endif()

if(${CLIB} STREQUAL newlib-nano OR ${CLIB} STREQUAL redlib)
  # printf float (newlib-nano and redlib only)
  if(${PRINTF_FLOAT})
    message(STATUS "printf float enabled.")
  else()
    message(STATUS "printf float disabled.")
  endif()
endif()
if(${CLIB} STREQUAL newlib-nano)
  # sprintf float (newlib-nano only)
  if(${SPRINTF_FLOAT})
    message(STATUS "sprintf float enabled.")
  else()
    message(STATUS "sprintf float disabled.")
  endif()
endif()
if(${CLIB} STREQUAL redlib)
  # char printf (redlib only)
  if(${CHAR_PRINTF})
    message(STATUS "char printf enabled.")
  else()
    message(STATUS "char printf disabled.")
  endif()
endif()

# cmake_minimum_required(VERSION 3.10)

# set(CMAKE_TOOLCHAIN_FILE "./arm-none-eabi-gcc.cmake")
# set(CMAKE_VERBOSE_MAKEFILE ON)


# # Supported C libraries
# set(SUPPORTED_CLIBS newlib newlib-nano redlib)

# # Supported hosting settings
# set(SUPPORTED_HOSTING none nohost semihosting)

# # Supported target devices
# set(SUPPORTED_DEVICES LPC4337)

# project(cursada_mc2)

# set(EXECUTABLE ${PROJECT_NAME}.axf)
# set(LINKER_FILE ${CMAKE_SOURCE_DIR}/ld/cursada_Debug.ld)
# set(LPCOPEN_CHIP_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_chip_43xx")
# set(LPCOPEN_BOARD_INCLUDE_PATHS "C:/Users/agust/Documents/MCUXpressoIDE_11.7.0_9198/workspace/lpc_board_nxp_lpcxpresso_4337")

# enable_language(C ASM)
# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_EXTENSIONS OFF)


# set(CLIB redlib)
# set(HOSTING nohost)

# # List of source files
# set(SRC_FILES src/main.c)

# # Build the executable based on the source files
# add_executable(${EXECUTABLE} ${SRC_FILES})

# target_include_directories(${EXECUTABLE} PRIVATE
#         inc
#         src
#         ${LPCOPEN_CHIP_INCLUDE_PATHS}/inc
#         ${LPCOPEN_CHIP_INCLUDE_PATHS}/inc/config_43xx
#         ${LPCOPEN_BOARD_INCLUDE_PATHS}/inc
#         )


# # Compiler options
# # target_compile_options(${EXECUTABLE} PRIVATE
# #     -std=c99
# #     -mcpu=cortex-m4
# #     -mfpu=fpv4-sp-d16
# #     -fno-common
# #     -pedantic
# #     -Wall
# #     -Wconversion
# #     -Wcast-align
# #     -Wcast-qual
# #     -Wlogical-op
# #     -Wshadow
# #     -Wsign-conversion
# #     -Wfloat-equal
# #     -c
# #     -fmessage-length=0
# #     -fno-builtin
# #     -ffunction-sections
# #     -fdata-sections
# #     -fsingle-precision-constant
# #     -v
# #     -fmerge-constants
# #     -mfloat-abi=softfp
# #     -fstack-usage
# #     -specs=redlib.specs
# #     -mthumb
# #     -Wall
# #     -O0
# #     -g3
# #     )

# set(C_FLAGS
# -std=c99
# -mcpu=cortex-m4
# -mfpu=fpv4-sp-d16
# -fno-common
# -pedantic
# -Wall
# -Wconversion
# -Wcast-align
# -Wcast-qual
# -Wlogical-op
# -Wshadow
# -Wsign-conversion
# -Wfloat-equal
# # -c
# -fmessage-length=0
# -fno-builtin
# -ffunction-sections
# -fdata-sections
# -fsingle-precision-constant
# -v
# -fmerge-constants
# -mfloat-abi=softfp
# -fstack-usage
# -specs=redlib.specs
# -mthumb
# -Wall
# -O0
# -g3
#     # "-std=gnu11"
#     # "-mfpu=fpv4-sp-d16"
#     # "-mcpu=cortex-m4"
#     # "-mthumb -mabi=aapcs"
#     # "-mfloat-abi=hard -mfpu=fpv4-sp-d16"
#     # "-ffunction-sections -fdata-sections -fno-strict-aliasing"
#     # "-fno-builtin -fshort-enums"
#     # "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-main"
# )

# set(ASM_FLAGS "-DDEBUG 
# -D__CODE_RED 
# -DCORE_M4 
# -D__USE_LPCOPEN 
# -DNO_BOARD_LIB 
# -D__LPC43XX__ 
# -D__REDLIB__ 
# -mcpu=cortex-m4 
# -mfpu=fpv4-sp-d16 
# -mfloat-abi=softfp 
# -mthumb 
# -D__REDLIB__ 
# -specs=redlib.specs")

# # set(LD_FLAGS
# #     -nostdlib
# #     -L${LPCOPEN_CHIP_INCLUDE_PATHS}/Debug 
# #     -Xlinker -Map=${PROJECT_NAME}.map
# #     -mcpu=cortex-m4 
# #     -mfpu=fpv4-sp-d16 
# #     -mfloat-abi=softfp 
# #     -mthumb
# #     -T ${LINKER_FILE}
# #     -llpc_chip_43xx
# #     -Xlinker -print-memory-usage 
# #     -Xlinker --cref 
# #     -Xlinker --gc-sections
# # )
# target_link_directories(${EXECUTABLE} PRIVATE
#     ${CMAKE_SOURCE_DIR}/ld/
# )

# target_link_options(${EXECUTABLE} PRIVATE
#     -nostdlib
#     -L${LPCOPEN_CHIP_INCLUDE_PATHS}/Debug 
#     -Xlinker -Map=${PROJECT_NAME}.map
#     -mcpu=cortex-m4 
#     -mfpu=fpv4-sp-d16 
#     -mfloat-abi=softfp 
#     -mthumb
#     -T ${LINKER_FILE}
#     -llpc_chip_43xx
#     -Wl,-print-memory-usage 
#     -Wl,--cref 
#     -Wl,--gc-sections
# )

# add_compile_definitions(-DDEBUG
#         -D__CODE_RED
#         -DCORE_M4
#         -D__USE_LPCOPEN
#         -DNO_BOARD_LIB
#         -D__LPC43XX__
#         -D__REDLIB__)

# list(JOIN C_FLAGS " " C_FLAGS)
# list(JOIN ASM_FLAGS " " ASM_FLAGS)
# list(JOIN LD_FLAGS " " LD_FLAGS)

# set(CMAKE_ASM_FLAGS_DEBUG "${ASM_FLAGS}")
# set(CMAKE_ASM_FLAGS_RELEASE "${ASM_FLAGS}")
# set(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -DDEBUG -g -Og")
# set(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -DNDEBUG -O3 -g3 -flto")
# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LD_FLAGS} -g -Og")
# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LD_FLAGS} -O3 -g3 -flto")


# # Optional: issue a message to be sure it uses the correct toolchain file.
# message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")


# add_library(lpc_chip_43xx  ${LPCOPEN_CHIP_INCLUDE_PATHS})
# add_library(lpc_board_nxp_lpcxpresso_4337  ${LPCOPEN_BOARD_INCLUDE_PATHS})


# set_target_properties(lpc_chip_43xx PROPERTIES LINKER_LANGUAGE C)
# set_target_properties(lpc_board_nxp_lpcxpresso_4337 PROPERTIES LINKER_LANGUAGE C)

# add_subdirectory(${PROJECT_SOURCE_DIR}/src     build/src)

# # # Linker options
# # target_link_options(${EXECUTABLE} PRIVATE
# #         -nostdlib
# #         -L${LPCOPEN_CHIP_INCLUDE_PATHS}/Debug 
# #         -Xlinker -Map=${PROJECT_NAME}.map
# #         -mcpu=cortex-m4 
# #         -mfpu=fpv4-sp-d16 
# #         -mfloat-abi=softfp 
# #         -mthumb
# #         -T ${LINKER_FILE}
# #         -llpc_chip_43xx
# #         -Xlinker -print-memory-usage 
# #         -Xlinker --cref 
# #         -Xlinker --gc-sections
# #         )

# target_link_libraries( ${EXECUTABLE}
#     src
#     lpc_chip_43xx
#     lpc_board_nxp_lpcxpresso_4337
#     )

# # # Optional: Print executable size as part of the post build process
# # add_custom_command(TARGET ${EXECUTABLE}
# #         POST_BUILD
# #         COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE})
