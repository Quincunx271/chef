add_library(chef INTERFACE)
add_library(chef::chef ALIAS chef)
target_include_directories(chef
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(chef
  INTERFACE
    $<BUILD_INTERFACE:Chef::CTRE>
)

# Tests

if(BUILD_TESTING)
  find_package(Catch2 2.4.0 REQUIRED)

  add_executable(test.chef
    catch_main.test.cpp
    # sources
    chef/chef.hpp
    chef/chef.test.cpp
  )
  target_link_libraries(test.chef
    PRIVATE
      chef::chef
      Catch2::Catch2
  )

  include(Catch)

  catch_discover_tests(test.chef
    EXTRA_ARGS $<$<BOOL:${CHEF_TEST_COLOR}>:--use-colour=yes>
  )
endif()

# Install

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/chef")

install(TARGETS chef
  EXPORT chef-Targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(EXPORT chef-Targets
  FILE chefTargets.cmake
  NAMESPACE chef::
  DESTINATION "${INSTALL_CONFIGDIR}"
)

install(DIRECTORY chef/
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/chef"
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/chefConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/chefConfigVersion.cmake"
  DESTINATION "${INSTALL_CONFIGDIR}"
)
