#ifndef FB_COMBY_ENCODING_LOCALE_HPP
#define FB_COMBY_ENCODING_LOCALE_HPP
#include <cstddef>
#include <cstdlib>
#include <cuchar>
#ifndef __STDC_UTF_32__
#error "mbrtoc32() & c32rtomb() must use UTF-32"
#endif
#include <memory>
#include <exception>
#include <span>
#include "fb/comby/encoding.hpp"

namespace fb::comby::encoding {
	// corosponds to the current C/C++ locale
	struct locale {
		using unit_type = char;
		using code_type = char32_t;
		using state_type = std::mbstate_t;

		static constexpr std::size_t max_units = MB_LEN_MAX;
		static constexpr std::size_t max_codes = 1;

		static encode_result<locale> encode(state_type& state,
						    std::span<code_type const> src,
						    std::span<unit_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (!is_unicode_scalar(src[0])) {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			}

			auto ret = std::c32rtomb(dst.data(), src[0], std::addressof(state));

			switch(ret) {
				case static_cast<std::size_t>(0): std::terminate();
				case static_cast<std::size_t>(-1): return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
				default: return {result_code::OK, src.subspan(0, 1), dst.subspan(0, ret)};
			}
		}

		static decode_result<locale> decode(state_type& state,
						    std::span<unit_type const> src,
						    std::span<code_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			}

			auto ret = std::mbrtoc32(dst.data(), src.data(), src.size(), std::addressof(state));

			switch(ret) {
				case static_cast<std::size_t>(0): return {result_code::OK};
				case static_cast<std::size_t>(-1): return {result_code::INVALID_ENCODING, src};
				case static_cast<std::size_t>(-2): std::terminate(); // not enough space, see encoding.hpp
				case static_cast<std::size_t>(-3): std::terminate(); // multi code point, not possible in UTF-32
				default: return {result_code::OK, src.subspan(0, ret), dst.subspan(0, 1)};
			}
		}
	};
}

#endif
