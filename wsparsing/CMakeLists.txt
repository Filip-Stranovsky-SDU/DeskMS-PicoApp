add_library(wsparsing
    ${CMAKE_CURRENT_SOURCE_DIR}/wsparsing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/api_call.cpp
)
target_include_directories(wsparsing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/../http_client

    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/lwip/src/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/lwip/src/include/lwip
)


target_link_libraries(wsparsing
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
    pico_stdlib

)

target_compile_features(wsparsing PUBLIC cxx_std_17)

# For the C-callable function
target_compile_definitions(wsparsing PUBLIC WSPARSING_EXPORTS)
