add_library(Display
    OLEDDisplay.cpp
    DisplayHandler.cpp
)
target_link_libraries(Display
    pico_stdlib
    hardware_i2c


)
target_compile_features(Display PUBLIC cxx_std_17)


target_include_directories(Display PUBLIC ${CMAKE_CURRENT_LIST_DIR})
