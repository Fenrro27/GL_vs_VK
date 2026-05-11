###############################################
# Compiler flags
###############################################

enable_language(CXX)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(WARNINGS "-Wall -Wextra")
    set(EXTRA_FLAGS "-std=c++11")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(WARNINGS "/W4 /WX /EHsc")
    set(WARNINGS_SUPPRESSED "/wd4714 /wd4018 /wd4098 /wd4099 /wd4201 /wd4290 /wd4611 /wd4996")
    set(EXTRA_FLAGS "${WARNINGS_SUPPRESSED} -O2")
endif()

if(NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${WARNINGS} ${EXTRA_FLAGS}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()


###############################################
# GLvsVK dependencies
###############################################

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "$ENV{VCPKG_ROOT}/installed/x64-linux")

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)


###############################################
# GLvsVK headers & sources
###############################################

file(GLOB_RECURSE GLvsVK_HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE GLvsVK_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)


###############################################
# GLvsVK Binary
###############################################

project(GLvsVK_EXAMPLE_2D CXX)

add_executable(GLvsVK_EXAMPLE_2D
    ${GLvsVK_SOURCES}
    ${GLvsVK_HEADERS}
)

target_include_directories(GLvsVK_EXAMPLE_2D PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(GLvsVK_EXAMPLE_2D PRIVATE
    Threads::Threads
    OpenGL::GL
    Vulkan::Vulkan
    GLEW::GLEW
    glfw
    glm::glm
)

set_target_properties(GLvsVK_EXAMPLE_2D PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
    RUNTIME_OUTPUT_NAME "GL_vs_VK"
)
