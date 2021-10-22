#ifndef FB_COMBY_ENCODING_ASCII_HPP
#define FB_COMBY_ENCODING_ASCII_HPP
#include <cstddef>
#include <span>
#include "fb/comby/encoding.hpp"

namespace fb::comby::encoding {
	struct ascii {
		using unit_type = char;
		using code_type = char32_t;
		struct state_type {};

		static constexpr std::size_t max_units = 1;
		static constexpr std::size_t max_codes = 1;

		static constexpr encode_result<ascii> encode(state_type&,
							     std::span<code_type const> src,
							     std::span<unit_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (!is_ascii(src[0])) {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			} else {
				dst[0] = src[0];
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			}
		}

		static constexpr decode_result<ascii> decode(state_type&,
							     std::span<unit_type const> src,
							     std::span<code_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (!is_ascii(src[0])) {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			} else {
				dst[0] = src[0];
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			}
		}
	};
}

#endif
