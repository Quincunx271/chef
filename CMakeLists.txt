cmake_minimum_required(VERSION 3.15)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(Chef
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "A lexing library"
)

include(is_root)
is_root(is_root_project)

set(CHEF_DEVELOPER_DEFAULTS "${is_root_project}" CACHE BOOL "Default all options to developer-friendly values")

option(BUILD_TESTING "Enable testing" ${CHEF_DEVELOPER_DEFAULTS})
option(CHEF_BUILD_DOCS "Build the documentation" OFF)
option(CHEF_TEST_COLOR "Force test color" OFF)
option(CHEF_WARNINGS_AS_ERRORS "Turn on -Werror or equivalent" OFF)
option(CHEF_DEVELOPMENT_SANITIZERS "Turn on sanitizers" ${CHEF_DEVELOPER_DEFAULTS})

if(BUILD_TESTING)
  enable_testing()
endif()

if(CHEF_DEVELOPER_DEFAULTS)
  include(pmm.cmake)
  pmm(CONAN)
endif()

if(CHEF_DEVELOPMENT_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GCC)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
  endif()
endif()

include(ChefFlags)
chef_add_warnings_target(_Chef_Warnings
  WERR ${CHEF_WARNINGS_AS_ERRORS}
)
add_library(Chef::Warnings ALIAS _Chef_Warnings)

add_subdirectory(src)

# Docs
if(CHEF_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Tests
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Chef")

install(TARGETS Chef _Chef_Warnings
  EXPORT Chef-Targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(EXPORT Chef-Targets
  FILE ChefTargets.cmake
  NAMESPACE Chef::
  DESTINATION "${INSTALL_CONFIGDIR}"
)

install(DIRECTORY include/Chef/
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Chef"
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ChefConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ChefConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ChefConfig.cmake"
  DESTINATION "${INSTALL_CONFIGDIR}"
)

if(CHEF_BUILD_DOCS)
  get_property(docs_dir TARGET docs PROPERTY SPHINX_OUTPUT_DIRECTORY)

  install(DIRECTORY ${docs_dir}
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
  )
endif()
