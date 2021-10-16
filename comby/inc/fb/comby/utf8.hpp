#ifndef FB_COMBY_UTF8_ENCODING_HPP
#define FB_COMBY_UTF8_ENCODING_HPP
#include <cstddef>
#include <ranges>
#include <algorithm>
#include "fb/comby/encoding.hpp"

namespace fb::comby::encoding {
	class utf8 {
	public:
		using unit_type = char8_t;
		using code_type = char32_t;
		struct state_type {};

		static constexpr std::size_t max_units = 4;
		static constexpr std::size_t max_codes = 1;

	private:
		static constexpr bool valid_cp(code_type cp) noexcept {
			return !(cp > 0xD800 && cp < 0xDBFF) // surrogate pair ranges
			    && !(cp > 0xDC00 && cp < 0xDFFF)
			    && cp <= 0x10FFFF; // maximum cp
		}

		enum unit_class : unit_t<utf8> {
			INVALID,
			LB_1, // ASCII
			TB, // trailing byte
			LB_2,
			LB_3,
			LB_4,
			_MAX
		};

		static constexpr auto unit_to_class = [](){
			auto arr = std::array<unit_class, 256>{};

			for (auto i = std::size_t{}; i < std::ranges::size(arr); ++i) {
				auto j = static_cast<unit_t<utf8>>(i);

				if (j < 0x80) {
					arr[i] = unit_class::LB_1;
				} else if ((j & 0b11000000) == 0b10000000) {
					arr[i] = unit_class::TB;
				} else if ((j & 0b11100000) == 0b11000000) {
					arr[i] = unit_class::LB_2;
				} else if ((j & 0b11110000) == 0b11100000) {
					arr[i] = unit_class::LB_3;
				} else if ((j & 0b11111000) == 0b11110000) {
					arr[i] = unit_class::LB_4;
				} else {
					arr[i] = INVALID;
				}
			}

			return arr;
		}();

		enum decode_state : unit_t<utf8> {
			DONE,
			ERROR,
			_3,
			_2,
			_1
		};

		static constexpr auto class_to_state = std::array{
			// INVALID, LB_1   TB,    LB_2,  LB_3,  LB_4
			   ERROR,   DONE,  ERROR, _1,    _2,    _3,    // DONE
			   ERROR,   ERROR, ERROR, ERROR, ERROR, ERROR, // ERROR
			   ERROR,   ERROR, _2,    ERROR, ERROR, ERROR, // _3
			   ERROR,   ERROR, _1,    ERROR, ERROR, ERROR, // _2
			   ERROR,   ERROR, DONE,  ERROR, ERROR, ERROR  // _1
		};

	public:
		constexpr utf8() = default;
		constexpr utf8(utf8 const&) = default;
		constexpr utf8(utf8&&) = default;

		constexpr utf8& operator=(utf8 const&) = default;
		constexpr utf8& operator=(utf8&&) = default;

		static constexpr encode_result<utf8> encode(state_type&,
							    std::span<code_type const> src,
							    std::span<unit_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK, src, dst};
			}

			auto const cp = src[0];

			if (!valid_cp(cp)) {
				return {result_code::INVALID_CODE_POINT, src, dst};
			} else if (cp < 0x80) {
				if (dst.empty()) {
					return {result_code::NOT_ENOUGH_STORAGE, src, dst};
				}

				dst[0] = static_cast<unit_type>(cp);

				return {result_code::OK, src.subspan(1), dst.subspan(1)};
			} else if (cp < 0x800) {
				if (dst.size() < 2) {
					return {result_code::NOT_ENOUGH_STORAGE, src, dst};
				}

				dst[0] = 0b11000000 | (static_cast<unit_type>(cp >> 6) & 0b00011111);
				dst[1] = 0b10000000 | (static_cast<unit_type>(cp) & 0b00111111);

				return {result_code::OK, src.subspan(1), dst.subspan(2)};
			} else if (cp < 0x10000) {
				if (dst.size() < 3) {
					return {result_code::NOT_ENOUGH_STORAGE, src, dst};
				}

				dst[0] = 0b11100000 | (static_cast<unit_type>(cp >> 12) & 0b00001111);
				dst[1] = 0b10000000 | (static_cast<unit_type>(cp >> 6) & 0b00111111);
				dst[2] = 0b10000000 | (static_cast<unit_type>(cp) & 0b00111111);

				return {result_code::OK, src.subspan(1), dst.subspan(3)};
			} else if (cp <= 0x10FFFF) {
				if (dst.size() < 4) {
					return {result_code::NOT_ENOUGH_STORAGE, src, dst};
				}

				dst[0] = 0b11110000 | (static_cast<unit_type>(cp >> 18) & 0b00000111);
				dst[1] = 0b10000000 | (static_cast<unit_type>(cp >> 12) & 0b00111111);
				dst[2] = 0b10000000 | (static_cast<unit_type>(cp >> 6) & 0b00111111);
				dst[3] = 0b10000000 | (static_cast<unit_type>(cp) & 0b00111111);

				return {result_code::OK, src.subspan(1), dst.subspan(4)};
			} else {
				return {result_code::INVALID_CODE_POINT, src, dst};
			}
		}

		static constexpr decode_result<utf8> decode(state_type&,
							    std::span<unit_type const> src,
							    std::span<code_type> dst) noexcept
		{
			if (src.empty()) {
				return {result_code::OK, src, dst};
			} else if (dst.empty()) {
				return {result_code::NOT_ENOUGH_STORAGE, src, dst};
			}

			auto s = decode_state::DONE;
			auto src_pos = std::ranges::begin(src);
			auto src_end = std::ranges::end(src);
			auto dst_pos = std::ranges::begin(dst);

			do {
				auto const c = unit_to_class[*src_pos];

				if (s == decode_state::DONE) {
					*dst_pos = (0xFF >> c) & *src_pos;
				} else {
					*dst_pos = (*dst_pos << 6) | static_cast<code_type>(*src_pos & 0x3F);
				}

				s = class_to_state[static_cast<unit_type>(unit_class::_MAX)
						 * static_cast<unit_type>(s)
						 + static_cast<unit_type>(c)];

				std::ranges::advance(src_pos, 1);
			} while(src_pos != src_end
			     && s != decode_state::DONE
			     && s != decode_state::ERROR);

			if (s == decode_state::DONE) {
				if (valid_cp(dst[0])) {
					return {result_code::OK, decltype(src){src_pos, src_end}, dst.subspan(1)};
				} else {
					return {result_code::INVALID_CODE_POINT, decltype(src){src_pos, src_end}, dst};
				}
			} else {
				return {result_code::INVALID_ENCODING, decltype(src){src_pos, src_end}, dst};
			}
		}
	};
}

#endif
