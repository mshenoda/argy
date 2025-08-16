cmake_minimum_required(VERSION 3.10)
project(argy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only library
add_library(argy INTERFACE)
target_include_directories(argy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Only add examples if this is the main project
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_EXAMPLES "Build example programs" ON)

    if(BUILD_EXAMPLES)
        add_executable(example_basic examples/example_basic.cpp)
        target_link_libraries(example_basic PRIVATE argy)
        add_executable(example_template examples/example_template.cpp)
        target_link_libraries(example_template PRIVATE argy)
        add_executable(example_named examples/example_named.cpp)
        target_link_libraries(example_named PRIVATE argy)
    endif()

    enable_testing()
    add_subdirectory(tests)
endif()
