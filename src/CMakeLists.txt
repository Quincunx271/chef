add_library(chef INTERFACE)
add_library(chef::chef ALIAS chef)
target_include_directories(chef
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(chef
  INTERFACE
    cxx_std_20
)

# Tests

if(BUILD_TESTING)
  find_package(Catch2 2.4.0 REQUIRED)

  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/catch_main.test.cpp"
    COMMAND ${CMAKE_COMMAND} -E echo "#define CATCH_CONFIG_MAIN" >> "${CMAKE_CURRENT_BINARY_DIR}/generated/catch_main.test.cpp"
    COMMAND ${CMAKE_COMMAND} -E echo "#include <catch2/catch.hpp>" >> "${CMAKE_CURRENT_BINARY_DIR}/generated/catch_main.test.cpp"
    VERBATIM
  )

  add_executable(test.chef
    "${CMAKE_CURRENT_BINARY_DIR}/generated/catch_main.test.cpp"
    # sources
    chef/_mdview.hpp
    chef/_mdview.test.cpp
    chef/chef.hpp
    chef/chef.test.cpp
    chef/dfa/dfa.hpp
    chef/dfa/dfa.test.cpp
    chef/dfa/epsilon.hpp
    chef/dfa/epsilon.test.cpp
    chef/dfa/nfa.hpp
    chef/dfa/nfa.test.cpp
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
