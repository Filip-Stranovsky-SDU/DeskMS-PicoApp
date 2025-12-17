# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(HelloWorld C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
# Add executable. Default name is the project name, version 0.1

file(GLOB BUTTONLED "${CMAKE_SOURCE_DIR}/ButtonLED/*.cpp")
add_subdirectory(wsparsing)
add_subdirectory(Display)


add_executable(HelloWorld HelloWorld.cpp
                tls_client/tls_common.c
                http_client/http_common.c
                tls_client/tls_payloads.c
                ${BUTTONLED}
)
message(STATUS "Executable was added")


pico_set_program_name(HelloWorld "HelloWorld")
pico_set_program_version(HelloWorld "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(HelloWorld 1)
pico_enable_stdio_usb(HelloWorld 1)


# Add the standard include files to the build
target_include_directories(HelloWorld PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_SOURCE_DIR}/tls_client
        
        ${CMAKE_SOURCE_DIR}/http_client
        ${CMAKE_SOURCE_DIR}/ButtonLED
        ${CMAKE_SOURCE_DIR}/..

)

target_compile_definitions(HelloWorld PRIVATE
        WIFI_SSID="POCO X3 NFC"
        WIFI_PASSWORD="xdxdxdxd"
        ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_OPTIONAL

)



target_link_libraries(HelloWorld
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        pico_mbedtls
        pico_stdlib
        hardware_rtc
        wsparsing
        Display
        
        )


# Ignore warnings from lwip code
set_source_files_properties(
        ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
        PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-result"
        )

pico_add_extra_outputs(HelloWorld)

# add_subdirectory(tests)

