#ifndef FB_COMBY_BIT_HPP
#define FB_COMBY_BIT_HPP
#include <climits>
#include <bit>
#include "fb/comby/concepts.hpp"
#include "fb/comby/algorithm.hpp"

namespace fb::comby::bit {
	template <concepts::integral I>
	constexpr I bswap(I const& i) noexcept {
		static_assert(sizeof(I) > 1, "bswap only makes sense for integrals larger than a byte");

		auto j = I{};

		algorithm::for_n<(sizeof(i) >> 1)>([&](std::size_t k) {
			j |= (i & (0xFF << (k * CHAR_BIT)))
			   | (i & (0xFF << (sizeof(I) - k * CHAR_BIT)));
		});

		return j;
	}

	template <std::endian E, typename I>
	constexpr I cond_bswap(I const& i) noexcept {
		if constexpr(E != std::endian::native) {
			return bswap(i);
		} else {
			return i;
		}
	}
}

#endif
