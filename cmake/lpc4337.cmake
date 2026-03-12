include_guard(GLOBAL)

set(LPC4337_CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=softfp
)

function(lpc4337_generate_linker_script out_var linker_dir)
    set(linker_script "${CMAKE_BINARY_DIR}/lpc4337.ld")
    file(GENERATE
        OUTPUT "${linker_script}"
        CONTENT "INCLUDE \"${linker_dir}/mem/mem.ld\"\nINCLUDE \"${linker_dir}/sections/sections.ld\"\n"
    )
    set(${out_var} "${linker_script}" PARENT_SCOPE)
endfunction()

function(lpc4337_configure_firmware_target target_name)
    set(map_file "${CMAKE_BINARY_DIR}/${target_name}.map")
    set(bin_file "${CMAKE_BINARY_DIR}/${target_name}.bin")
    set(hex_file "${CMAKE_BINARY_DIR}/${target_name}.hex")

    set_target_properties(${target_name} PROPERTIES
        OUTPUT_NAME "${target_name}"
        SUFFIX ".elf"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_link_libraries(${target_name} PRIVATE lpc43xx_common)
    target_link_options(${target_name} PRIVATE
        ${LPC4337_CPU_FLAGS}
        --specs=nano.specs
        --specs=nosys.specs
        -nostartfiles
        "-T${LPC4337_LINKER_SCRIPT}"
        "-Wl,-Map=${map_file}"
        -Wl,--gc-sections
        -Wl,--cref
        -Wl,--print-memory-usage
    )

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target_name}>
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target_name}> "${bin_file}"
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target_name}> "${hex_file}"
        VERBATIM
    )

    if(OPENOCD_EXECUTABLE)
        add_custom_target(flash_${target_name}
            COMMAND ${OPENOCD_EXECUTABLE}
                -f "${OPENOCD_CONFIG}"
                -c "init"
                -c "halt"
                -c "flash write_image erase \"$<TARGET_FILE:${target_name}>\""
                -c "verify_image \"$<TARGET_FILE:${target_name}>\""
                -c "reset run"
                -c "shutdown"
            DEPENDS ${target_name}
            USES_TERMINAL
            VERBATIM
        )
    else()
        add_custom_target(flash_${target_name}
            COMMAND ${CMAKE_COMMAND} -E echo "OpenOCD was not found in PATH. Install it or add it to PATH before flashing."
            COMMAND ${CMAKE_COMMAND} -E false
            DEPENDS ${target_name}
            VERBATIM
        )
    endif()
endfunction()
