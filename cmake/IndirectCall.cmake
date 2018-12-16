set(_indirect_call_file "${CMAKE_BINARY_DIR}/_indirect_call.cmake")

macro(indirect_call NAME)
  set(_indirect_call_argn "${ARGN}")
  list(JOIN _indirect_call_argn "\" \"" _indirect_call_args)
  file(WRITE "${_indirect_call_file}" "${NAME}(\"${_indirect_call_args}\")")
  include("${_indirect_call_file}")
endmacro()
