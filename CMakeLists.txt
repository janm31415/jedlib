cmake_minimum_required(VERSION 3.10)
project(jedlib)

if (APPLE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.7" CACHE STRING "Minimum macOS deployment version" FORCE)
endif(APPLE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_subdirectory(cpp-rrb)
add_subdirectory(jedlib)
add_subdirectory(jedlib.tests)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
