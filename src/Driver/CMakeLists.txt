file(GLOB FILES
  *.c
 )
 include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${LPCOPEN_CHIP_INCLUDE_PATHS}/inc
    )
    
 add_library(driver ${FILES})
# target_link_libraries: If you link with other libraries, list them here
target_link_libraries(
  driver
  src
  lpc_chip_43xx
)
 
# target_include_directories: Libraries need to publish their header files 
# so that you can import them in source code. This statement expresses where to find the files 
# - typically in an include directory of your projects.
# target_include_directories(
#   ../../inc/drivers 
# )