cmake_minimum_required(VERSION 3.5)

project (libpolaris)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

#
# Options
#
option(COMPILE_TESTS "Execute unit tests" ON)
option(WITH_ASAN     "Compile with ASAN" OFF)

#
# Setup build type 'Release vs Debug'
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

#
# Locate CPPUTest
#
if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

#
# Setup ASAN
#
if(WITH_ASAN)
  set(CMAKE_CX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()


include(${CMAKE_SOURCE_DIR}/cmake/SetEnv.cmake)

set(POLARIS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/polaris/polaris.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/polaris/evaluator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/polaris/environment.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/polaris/imports.cpp
)

set(HEADERS
    ${CMAKE_SOURCE_DIR}/polaris/fwd.hpp
    ${CMAKE_SOURCE_DIR}/polaris/imports.hpp
    ${CMAKE_SOURCE_DIR}/polaris/polaris.hpp
    ${CMAKE_SOURCE_DIR}/polaris/environment.hpp
    ${CMAKE_SOURCE_DIR}/polaris/evaluator.hpp
    ${CMAKE_SOURCE_DIR}/polaris/imports.hpp
)

set(SOURCES
    ${POLARIS_SOURCES}
)

#
# Configure Library
#
include(${CMAKE_SOURCE_DIR}/cmake/LibraryConfig.cmake)

#
# Configure Install
#
include(${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake)

#
# Setup polaris app
#
add_executable(polaris 
  app/main.cpp
  ${POLARIS_SOURCES}
)

target_include_directories(polaris
    PUBLIC 
        ${PROJECT_SOURCE_DIR}
)

#
# Install bin
#
install (TARGETS polaris
    DESTINATION bin)

#
# Tests
#
if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()