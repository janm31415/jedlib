cmake_minimum_required(VERSION 3.14)
project(jedlib)

# Only set a default deployment target for macOS; for iOS pass
# -DCMAKE_OSX_DEPLOYMENT_TARGET=... on the command line.
if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.7" CACHE STRING "Minimum macOS deployment version" FORCE)
endif ()

# Tests are executables; not useful on iOS.
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(_jedlib_tests_default OFF)
else ()
  set(_jedlib_tests_default ON)
endif ()
option(JEDLIB_BUILD_TESTS "Build the jedlib and immutable test executables" ${_jedlib_tests_default})

# On iOS keep CMake's default output dirs (e.g. Debug-iphoneos / Debug-iphonesimulator)
# so device and simulator builds don't overwrite each other.
if (NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
  set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_CURRENT_BINARY_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif ()

if (JEDLIB_BUILD_TESTS)
  add_subdirectory(cpp-rrb)            # immutable + immutable.tests
  add_subdirectory(jedlib)
  add_subdirectory(jedlib.tests)
else ()
  add_subdirectory(cpp-rrb/immutable)  # library only, no tests
  add_subdirectory(jedlib)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
