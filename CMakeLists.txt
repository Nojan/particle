cmake_minimum_required(VERSION 3.5)
project(Particle)
set (CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE sources_files sources/*)

include_directories(externals/glm)
include_directories(externals)
link_directories(externals/SDL2)

add_executable(particle ${sources_files})
target_link_libraries(particle SDL2)
if (CMAKE_HOST_WIN32)
    add_definitions(-DNOMINMAX)
    add_custom_command(
        TARGET particle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/externals/SDL2/SDL2.dll
            ${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll)
endif (CMAKE_HOST_WIN32)
