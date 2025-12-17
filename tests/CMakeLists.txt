cmake_minimum_required(VERSION 3.13)
project(unit_tests C CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(unit_tests
    tests_ws.c
    ../tls_client/tls_payloads.c
)
add_executable(unit_tests_dispatcher
    tests_Dispatcher.cpp
    ../wsparsing/wsparsing.cpp
)
target_include_directories(unit_tests_dispatcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ../wsparsing
    ../tls_client    
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ../wsparsing
    ../tls_client    
)
