cmake_minimum_required(VERSION 4.1)
project(lodepng CXX)

add_library(lodepng STATIC lodepng.cpp)
target_include_directories(lodepng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
