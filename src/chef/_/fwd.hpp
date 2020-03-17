#pragma once

#include <type_traits>

#define CHEF_I_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define CHEF_I_MOVE(...) static_cast<std::remove_cvref_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
