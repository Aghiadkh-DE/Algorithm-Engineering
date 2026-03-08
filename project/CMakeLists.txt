cmake_minimum_required(VERSION 4.1)
project(Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)


if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL /fp:fast")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    add_compile_options(/W4)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
    add_compile_options(-Wall -Wextra -pedantic)
endif()


add_library(project_lib STATIC
        src/image/colorspace.cpp
        src/image/image_gray.cpp
        src/io/ppm_reader.cpp
        src/io/ppm_writer.cpp
        src/io/token_reader.cpp
        src/ops/background_estimate.cpp
        src/ops/background_remove.cpp
        src/ops/border_cleanup.cpp
        src/ops/contrast_stretch.cpp
        src/ops/denoise_median.cpp
        src/ops/morphology.cpp
        src/ops/threshold_nick.cpp
        src/ops/threshold_otsu.cpp
        src/ops/threshold_proposed.cpp
        src/ops/threshold_sauvola.cpp
        src/ops/threshold_su.cpp
        src/util/args_parser.cpp
        src/util/logging.cpp
        src/util/timing.cpp
)

target_include_directories(project_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(project_lib PUBLIC OpenMP::OpenMP_CXX)

add_executable(project_executable
        src/main/main.cpp
)

target_link_libraries(project_executable PRIVATE project_lib OpenMP::OpenMP_CXX)

if(BUILD_TESTING)
    find_package(Catch2 3 QUIET)
    if(Catch2_FOUND)
        add_subdirectory(tests)
    else()
        message(STATUS "Catch2 not found, skipping project tests")
    endif()
endif()
