#pragma once

#include <tl/tl.hpp>
#include <utility>

#define CHEF_FWD TL_FWD

#define CHEF_MOVE(...) static_cast<::std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
