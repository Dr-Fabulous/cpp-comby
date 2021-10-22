#ifndef FB_COMBY_ALGORITHM_HPP
#define FB_COMBY_ALGORITHM_HPP
#include <cstddef>
#include <type_traits>
#include <functional>

namespace fb::comby::algorithm {
	template <std::size_t N, typename F>
	constexpr void for_n(F&& f) noexcept(noexcept(std::invoke(std::declval<F>(), std::declval<std::size_t>()))) {
		[&]<std::size_t... Is> (std::index_sequence<Is...>) {
			(std::invoke(std::forward<F>(f), Is), ...);
		}(std::make_index_sequence<N>());
	}
}

#endif
