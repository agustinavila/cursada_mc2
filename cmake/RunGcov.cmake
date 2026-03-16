if(NOT DEFINED GCOV_EXECUTABLE)
    message(FATAL_ERROR "GCOV_EXECUTABLE is required.")
endif()

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR is required.")
endif()

file(GLOB_RECURSE GCNO_FILES "${BINARY_DIR}/*.gcno")

if(NOT GCNO_FILES)
    message(FATAL_ERROR "No .gcno files were found under ${BINARY_DIR}.")
endif()

foreach(GCNO_FILE IN LISTS GCNO_FILES)
    execute_process(
        COMMAND "${GCOV_EXECUTABLE}" -b -c "${GCNO_FILE}"
        WORKING_DIRECTORY "${BINARY_DIR}"
        RESULT_VARIABLE GCOV_RESULT
        OUTPUT_VARIABLE GCOV_STDOUT
        ERROR_VARIABLE GCOV_STDERR
    )

    string(STRIP "${GCOV_STDOUT}" GCOV_STDOUT)
    string(STRIP "${GCOV_STDERR}" GCOV_STDERR)

    if(GCOV_STDOUT)
        message(STATUS "${GCOV_STDOUT}")
    endif()

    if(GCOV_STDERR)
        message(STATUS "${GCOV_STDERR}")
    endif()

    if(NOT GCOV_RESULT EQUAL 0)
        message(FATAL_ERROR "gcov failed for ${GCNO_FILE}")
    endif()
endforeach()
