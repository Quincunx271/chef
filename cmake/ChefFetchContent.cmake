include(FetchContent)
include(IndirectCall)

function(chef_fetch_content)
  set(options)
  set(args NAME ON_POPULATE)
  set(multi_args EXTRA_ARGS)

  cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${args}" "${multi_args}")

  string(TOLOWER "${ARG_NAME}" lname)

  FetchContent_Declare(
    "${ARG_NAME}"
    "${ARG_UNPARSED_ARGUMENTS}"
  )

  FetchContent_GetProperties("${ARG_NAME}")
  if(NOT "${${lname}_POPULATED}")
    FetchContent_Populate("${ARG_NAME}")

    indirect_call("${ARG_ON_POPULATE}" "${lname}" ${ARG_EXTRA_ARGS})
  endif()
endfunction()
