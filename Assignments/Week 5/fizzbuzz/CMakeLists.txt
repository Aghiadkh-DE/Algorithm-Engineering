cmake_minimum_required(VERSION 4.1)
project(fizzbuzz LANGUAGES CXX)

add_library(fizzbuzz STATIC
    src/fizzbuzz.cpp
)
target_include_directories(fizzbuzz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(fizzbuzz_tests
    tests/test_fizzbuzz.cpp
)
target_link_libraries(fizzbuzz_tests PRIVATE fizzbuzz catch)

add_test(NAME fizzbuzz_tests COMMAND fizzbuzz_tests)
