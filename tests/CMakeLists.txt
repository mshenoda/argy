cmake_minimum_required(VERSION 3.20)

# Download doctest header if not already present
set(DOCTEST_URL https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h)
set(DOCTEST_INCLUDE_DIR ${CMAKE_BINARY_DIR}/external/doctest)
file(MAKE_DIRECTORY ${DOCTEST_INCLUDE_DIR})
set(DOCTEST_HEADER ${DOCTEST_INCLUDE_DIR}/doctest.h)
if(NOT EXISTS "${DOCTEST_HEADER}")
    file(DOWNLOAD
        ${DOCTEST_URL}
        "${DOCTEST_HEADER}"
        SHOW_PROGRESS
    )
endif()

add_executable(test_argy test_argy.cpp)
target_link_libraries(test_argy PRIVATE argy)
target_include_directories(test_argy PRIVATE ${CMAKE_SOURCE_DIR}/include ${DOCTEST_INCLUDE_DIR})

include(CTest)
add_test(NAME argy_tests COMMAND test_argy)
