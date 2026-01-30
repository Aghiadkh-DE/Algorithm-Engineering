cmake_minimum_required(VERSION 4.1)
project(quantize C)

add_library(quantize STATIC
    blur.c
    kmeans.c
    libimagequant.c
    mediancut.c
    mempool.c
    nearest.c
    pam.c
)
target_include_directories(quantize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(quantize PUBLIC OpenMP::OpenMP_C)
