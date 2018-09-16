cmake_minimum_required(VERSION 3.5)
set( TARGET_NAME Particle )
project( ${TARGET_NAME} )
set (CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE sources_files sources/*)

include_directories(externals/glm)
include_directories(externals)
link_directories(externals/SDL2)

add_executable(${TARGET_NAME} ${sources_files})
if( EMSCRIPTEN )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/html )

	set( CMAKE_EXECUTABLE_SUFFIX ".html" )
	#add_definitions( -s USE_SDL=2 -s LEGACY_GL_EMULATION=1 -std=c++1y )
	add_definitions( -s USE_SDL=2 )
	# RELEASE
	set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-O2 -s USE_SDL=2 --preload-file ${CMAKE_SOURCE_DIR}/shaders@../shaders" )
	# DEV
	# set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-s USE_SDL=2 --preload-file ${CMAKE_SOURCE_DIR}/shaders@../shaders" )
	set_property( TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14 )
	target_link_libraries( ${TARGET_NAME} ${DEPENDENCIES_LIBRARIES} )
else()
	target_link_libraries(${TARGET_NAME} SDL2)
endif()

if (CMAKE_HOST_WIN32 AND NOT EMSCRIPTEN)
    add_definitions(-DNOMINMAX)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/externals/SDL2/SDL2.dll
            ${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll)
endif (CMAKE_HOST_WIN32)
