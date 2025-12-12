add_library(wsparsing wsparsing.cpp)

target_include_directories(wsparsing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/commands
)

target_compile_features(wsparsing PUBLIC cxx_std_23)

# For the C-callable function
target_compile_definitions(wsparsing PUBLIC WSPARSING_EXPORTS)
