###############################################
# CMake options
###############################################

cmake_minimum_required (VERSION 3.5.1 FATAL_ERROR)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

###############################################
# GLvsVK
###############################################

add_subdirectory(project)
