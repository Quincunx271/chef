#include "./derivative.hpp"

#include <algorithm>
#include <numeric>

#include <chef/_/fwd.hpp>
#include <chef/_/overload.hpp>
#include <chef/_/ranges.hpp>

using chef::detail::overload;
namespace ranges = std::ranges;
namespace views = ranges::views;
using ranges::begin;
using ranges::end;

namespace {
	template <typename Range>
		requires ranges::range<std::remove_cvref_t<Range>> //
			chef::re concat_all(chef::re first, Range&& rest)
		{
			return std::accumulate(begin(rest), end(rest), CHEF_MOVE(first),
				[](chef::re lhs, chef::re rhs) { return CHEF_MOVE(lhs) << CHEF_MOVE(rhs); });
		}
}

namespace chef {
	auto derivative(chef::re const& re, std::string_view str) -> chef::re
	{
		return std::accumulate(begin(str), end(str), re, [] TL(chef::derivative(CHEF_FWD(_1), _2)));
	}

	auto derivative(chef::re const& re_in, char c) -> chef::re
	{
		return std::visit(
			overload{
				[c](chef::re_lit lit) -> re {
					if (!lit.value.empty() && lit.value.front() == c) {
						lit.value.erase(lit.value.begin());
						return re(CHEF_MOVE(lit));
					} else {
						return re();
					}
				},
				[c](chef::re_empty) -> re { return re(); },
				[c](chef::re_char_class) -> re { throw 1; },
				[c](re_union alt) -> re {
					return std::transform_reduce(begin(alt.pieces), end(alt.pieces), chef::re(),
						std::bit_or{}, [c] TL(chef::derivative(CHEF_FWD(*_1), c)));
				},
				[c](re_cat cat) -> re {
					if (cat.pieces.empty()) return chef::derivative(chef::re(chef::re_lit{""}), c);

					auto rest = cat.pieces.size() > 2
						? chef::re(re_cat{std::vector(cat.pieces.begin() + 1, cat.pieces.end())})
						: chef::re(*cat.pieces[1]);

					auto res = chef::derivative(*cat.pieces.front(), c) << rest;
					if (cat.pieces.front()->is_vanishable()) res = res | chef::derivative(rest, c);
					return res;
				},
				[c](re_star star) -> re {
					return chef::derivative(*star.value, c) << re(CHEF_MOVE(star));
				},
			},
			re_in.value);
	}

	bool re_derivative_engine::matches(chef::re const& re, std::string_view str)
	{
		return chef::derivative(re, str).is_vanishable();
	}
}
