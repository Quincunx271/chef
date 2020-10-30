#pragma once

#include <type_traits>

#include <chef/_/fwd.hpp>
#include <chef/_/tag.hpp>

namespace chef {
    template <typename Fn>
    class _deduce {
        [[no_unique_address]] Fn fn_;

    public:
        using _chef_deduce_fn_tag = void;

        template <typename F>
        explicit constexpr _deduce(F&& fn)
            : fn_ {CHEF_I_FWD(fn)}
        { }

        template <typename T>
            constexpr operator T() && requires (!std::is_reference_v<T>)
        {
            return fn_(chef::_tag<T>);
        }

        template <typename T>
        constexpr operator T&() &&
        {
            return fn_(chef::_tag<T&>);
        }

        template <typename T>
        constexpr operator T const &() &&
        {
            return fn_(chef::_tag<T const&>);
        }

        template <typename T>
        constexpr operator T&&() &&
        {
            return fn_(chef::_tag<T&&>);
        }

        template <typename T>
        constexpr operator T const &&() &&
        {
            return fn_(chef::_tag<T const&&>);
        }
    };

    template <typename F>
    explicit _deduce(F) -> _deduce<F>;

    template <typename T>
    concept _is_deduce = requires(T val)
    {
        typename std::remove_cvref_t<decltype(val)>::_chef_deduce_fn_tag;
    };
}
