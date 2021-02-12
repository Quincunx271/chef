find_package(Catch2 REQUIRED)

file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "*.test.cpp")

add_executable(test.Chef ${test_sources})
target_link_libraries(test.Chef
  PRIVATE
    Chef::Chef
    Catch2::Catch2
    Chef::Warnings
)
target_compile_options(test.Chef
  PRIVATE
    ${compile_options}
)
target_link_options(test.Chef
  PRIVATE
    ${link_options}
)

include(Catch)

catch_discover_tests(test.Chef
  EXTRA_ARGS $<$<BOOL:${CHEF_TEST_COLOR}>:--use-colour=yes>
)
