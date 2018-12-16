cmake_minimum_required(VERSION 3.12)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(chef
  VERSION 0.0.1
  LANGUAGES C CXX
  DESCRIPTION "A lexer library"
)

include(is_root)
is_root(is_root_project)

set(CHEF_DEVELOPER_DEFAULTS "${is_root_project}" CACHE BOOL "Default all options to developer-friendly values")

option(BUILD_TESTING "Enable testing" ${CHEF_DEVELOPER_DEFAULTS})
option(CHEF_TEST_COLOR "Force test color" OFF)

if(BUILD_TESTING)
  enable_testing()
endif()

if(is_root_project)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Set up dependencies
include(pmm.cmake)
pmm(CONAN)

# Set up warnings / similar flags
set(MSVC_flags /permissive-)
set(Clang_flags -Wall -Wextra -Wpedantic)
set(GNU_flags -Wall -Wextra -Wpedantic)

add_compile_options(${${CMAKE_CXX_COMPILER_ID}_flags})

# Project
add_subdirectory(src)
