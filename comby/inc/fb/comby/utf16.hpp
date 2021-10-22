#ifndef CPP_COMBY_ENCODING_UTF16_HPP
#define CPP_COMBY_ENCODING_UTF16_HPP
#include <cstddef>
#include <bit>
#include <span>
#include "fb/comby/encoding.hpp"
#include "fb/comby/bit.hpp"

namespace fb::comby::encoding {
	template <std::endian E, typename U = char16_t>
	struct base_utf16 {
		using unit_type = U;
		using code_type = char32_t;
		struct state_type {};

		static constexpr std::size_t max_units = 2;
		static constexpr std::size_t max_codes = 1;

		static constexpr encode_result<base_utf16> encode(state_type&,
								  std::span<code_type const> src,
								  std::span<unit_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (src[0] < 0xD800u) {
				if (dst.empty()) {
					return {result_code::NOT_ENOUGH_STORAGE};
				}

				dst[0] = bit::cond_bswap<E, unit_type>(src[0]);

				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			} else if (src[0] <= 0xDFFFu) {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			} else if (src[0] <= 0xFFFFu) {
				if (dst.empty()) {
					return {result_code::NOT_ENOUGH_STORAGE};
				}

				dst[0] = bit::cond_bswap<E, unit_type>(src[0]);

				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			} else if (src[0] <= 0x10FFFFu) {
				if (dst.size() < 2) {
					return {result_code::NOT_ENOUGH_STORAGE};
				}

				auto const cp = src[0] - 0x10000u;

				dst[0] = bit::cond_bswap<E, unit_type>((cp >> 10) + 0XD800u);
				dst[1] = bit::cond_bswap<E, unit_type>((cp & 0x3FF) + 0xDC00u);

				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 2)};
			} else {
				return {result_code::INVALID_CODE_POINT, src.subspan(0, 1)};
			}
		}

		static constexpr decode_result<base_utf16> decode(state_type&,
								  std::span<unit_type const> src,
								  std::span<code_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK};
			} else if (dst.empty()) {
				return {result_code::NOT_ENOUGH_STORAGE};
			}

			auto const u1 = bit::cond_bswap<E>(src[0]);

			if (u1 < 0xD800u) {
				dst[0] = u1;
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			} else if (u1 <= 0xDBFFu) {
				if (src.size() < 2) {
					return {result_code::INVALID_ENCODING, src.subspan(0, 1)};
				}

				auto const u2 = bit::cond_bswap<E>(src[1]);

				if (u2 < 0xDC00u || u2 > 0xDFFFu) {
					return {result_code::INVALID_ENCODING, src.subspan(0, 2)};
				}

				dst[0] = 0x10000
				       + (((u1 - 0xD800u) << 10)
				       | (u2 - 0xDC00u));

				if (!is_unicode_scalar(dst[0])) {
					return {result_code::INVALID_CODE_POINT, src.subspan(0, 2), dst.subspan(0, 1)};
				} else {
					return {result_code::OK, src.subspan(0, 2), dst.subspan(0, 1)};
				}
			} else if (u1 <= 0xDFFFu) {
				return {result_code::INVALID_ENCODING, src.subspan(0, 1)};
			} else {
				dst[0] = u1;
				return {result_code::OK, src.subspan(0, 1), dst.subspan(0, 1)};
			}
		}
	};

	using utf16_le = base_utf16<std::endian::little>;
	using utf16_be = base_utf16<std::endian::big>;
	//using utf16_wnd = base_utf16<std::endian::little, wchar_t>; // disgusting
	using utf16 = base_utf16<std::endian::native>;
}

#endif
