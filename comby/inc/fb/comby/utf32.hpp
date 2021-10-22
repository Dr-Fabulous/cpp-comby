#ifndef FB_COMBY_ENCODING_UTF32_HPP
#define FB_COMBY_ENCODING_UTF32_HPP
#include <cstddef>
#include "fb/comby/encoding.hpp"
#include "fb/comby/bit.hpp"

namespace fb::comby::encoding {
	template <std::endian E>
	struct base_utf32 {
		using unit_type = char32_t;
		using code_type = char32_t;
		struct state_type {};

		static constexpr std::size_t max_units = 1;
		static constexpr std::size_t max_codes = 1;

		static constexpr encode_result<base_utf32> encode(state_type&,
								  std::span<code_type const> src,
								  std::span<unit_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (!is_unicode_scalar(src[0])) {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			} else {
				dst[0] = bit::cond_bswap<E>(src[0]);
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			}
		}

		static constexpr decode_result<base_utf32> decode(state_type&,
								  std::span<unit_type const> src,
								  std::span<code_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			}

			auto const cp = bit::cond_bswap<E>(src[0]);

			if (!is_unicode_scalar(cp)) {
				return {result_code::INVALID_ENCODING, src.subspan(0, 1)};
			} else {
				dst[0] = cp;
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			}
		}
	};

	using utf32_le = base_utf32<std::endian::little>;
	using utf32_be = base_utf32<std::endian::big>;
	using utf32 = base_utf32<std::endian::native>;
}

#endif
