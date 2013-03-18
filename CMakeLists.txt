cmake_minimum_required(VERSION 2.8)
project(mbm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Wall -Werror")

# Version information
set(MBM_VERSION_MAJOR 1)
set(MBM_VERSION_MINOR 0)
if (NOT MBM_BUILD_NUMBER)
    set(MBM_BUILD_NUMBER 0)
endif()
set(MBM_PATCH_LEVEL 0)

configure_file(
    "${PROJECT_SOURCE_DIR}/src/mbm_config.h.in"
    "${PROJECT_BINARY_DIR}/mbm_config.h"
)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}")

# Dependencies for which we have CMake Find modules
find_package(Boost 1.50 COMPONENTS program_options)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

enable_testing()
add_subdirectory(src)
