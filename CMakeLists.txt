# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FAMILY rp2040)
set(BOARD pico_sdk)

if(NOT DEFINED TINYUSB_DIR)
    set(TINYUSB_DIR ${PICO_SDK_PATH}/lib/tinyusb)
    message(STATUS "TINYUSB_DIR = ${TINYUSB_DIR}")
endif()

set(TARGET_FILES
    ${CMAKE_CURRENT_LIST_DIR}/flash_access.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FlashAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UsbCDC.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StringParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Stack.cpp
    ${TINYUSB_DIR}/examples/device/cdc_dual_ports/src/usb_descriptors.c
    ${TINYUSB_DIR}/src/tusb.c
)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(flash_access C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(flash_access ${TARGET_FILES})

target_compile_options(flash_access PRIVATE -O0)

pico_set_program_name(flash_access "flash_access")
pico_set_program_version(flash_access "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(flash_access 1)
pico_enable_stdio_usb(flash_access 0)

# Add the standard library to the build
target_link_libraries(flash_access
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

# Add the standard include files to the build
target_include_directories(flash_access PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${TINYUSB_DIR}/examples/device/cdc_dual_ports/src
        ${TINYUSB_DIR}/hw
        ${TINYUSB_DIR}/src/class/cdc
        ${TINYUSB_DIR}/src/common
)

pico_add_extra_outputs(flash_access)