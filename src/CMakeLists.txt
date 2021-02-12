file(GLOB_RECURSE sources "chef/*.hpp" "chef/*.cpp")

add_library(Chef ${sources})
add_library(Chef::Chef ALIAS Chef)
target_include_directories(Chef
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(Chef
  PUBLIC
    cxx_std_20
)
target_link_libraries(Chef PRIVATE Chef::Warnings)
