# Sets up compilation flags for Chef

function(chef_add_warnings_target TARGET)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "WERR" "")

  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  set(werr ${ARG_WERR})

  set(MSVC_WARNINGS /permissive- /W4
    $<$<BOOL:${werr}>:/WX>
  )
  set(CLANG_WARNINGS
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${werr}>:-Werror>
  )
  set(GCC_WARNINGS
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${werr}>:-Werror>
  )

  add_library(${TARGET} INTERFACE)
  target_compile_options(${TARGET} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>
    $<$<CXX_COMPILER_ID:GNU>:${GCC_WARNINGS}>
  )
endfunction()
