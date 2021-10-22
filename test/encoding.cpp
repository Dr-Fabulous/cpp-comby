#include <cassert>
#include <cstddef>
#include <utility>
#include <functional>
#include <ranges>
#include <tuple>
#include <array>
#include <string_view>
#include "fb/comby/encoding.hpp"
#include "fb/comby/ascii.hpp"
#include "fb/comby/utf8.hpp"
#include "fb/comby/utf16.hpp"
#include "fb/comby/utf32.hpp"
#include "fb/comby/locale.hpp"

using namespace std::literals;
using namespace fb::comby::encoding;

bool compare(auto const& a, auto const& b) noexcept {
	for (auto i = std::size_t{}, s = std::min(std::ranges::size(a), std::ranges::size(b)); i < s; ++i) {
		if (!a[i] || !b[i]) {
			return true;
		} else if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}

void test_utf8() noexcept {
	auto units = std::array<unit_t<utf8>, utf8::max_units>{};
	auto codes = std::array<code_t<utf8>, utf8::max_codes>{};
	auto state = state_t<utf8>{};

	auto const encode_decode = [&](std::u8string_view test_units, std::u32string_view test_codes) {
		assert(utf8::encode(state, test_codes, units));
		assert(compare(test_units, units));
		assert(utf8::decode(state, test_units, codes));
		assert(compare(test_codes, codes));
	};

	encode_decode(u8"A"sv, U"A"sv);
	encode_decode(u8"\u0400"sv, U"\u0400"sv);
	encode_decode(u8"\uFFFD"sv, U"\uFFFD"sv);
	encode_decode(u8"\U0010AAAA"sv, U"\U0010AAAA"sv);

	auto empty_units = std::array<unit_t<utf8>, 0>{};
	auto wrong_units = std::array<char8_t, 2>{0b11000011u, 0u};
	auto empty_codes = std::array<code_t<utf8>, 0>{};
	auto invalid_codes = std::array<char32_t, 1>{0x111111u};

	assert(utf8::encode(state, empty_codes, units).code == result_code::OK);
	assert(utf8::encode(state, empty_codes, empty_units).code == result_code::OK);
	assert(utf8::encode(state, empty_codes, units).code == result_code::OK);

	assert(utf8::decode(state, empty_units, codes).code == result_code::OK);
	assert(utf8::decode(state, empty_units, empty_codes).code == result_code::OK);
	assert(utf8::decode(state, wrong_units, codes).code == result_code::INVALID_ENCODING);

	assert(utf8::encode(state, std::array<char32_t, 1>{0xD8FF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf8::encode(state, std::array<char32_t, 1>{0xDCFF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf8::encode(state, std::array<char32_t, 1>{0x11FFFF}, units).code == result_code::INVALID_CODE_POINT);

	assert(utf8::decode(state, std::array<char8_t, 3>{0b11101101u, 0b10100000u, 0b10000001u}, codes).code == result_code::INVALID_CODE_POINT);
	assert(utf8::decode(state, std::array<char8_t, 3>{0b11101101u, 0b10110000u, 0b10000001u}, codes).code == result_code::INVALID_CODE_POINT);
}

void test_utf16() noexcept {
	auto units = std::array<unit_t<utf16>, utf16::max_units>{};
	auto codes = std::array<code_t<utf16>, utf16::max_codes>{};
	auto state = state_t<utf16>{};

	auto const encode_decode = [&](std::u16string_view test_units, std::u32string_view test_codes) {
		assert(utf16::encode(state, test_codes, units));
		assert(compare(test_units, units));
		assert(utf16::decode(state, test_units, codes));
		assert(compare(test_codes, codes));
	};

	encode_decode(u"A"sv, U"A"sv);
	encode_decode(u"\u0400"sv, U"\u0400"sv);
	encode_decode(u"\uFFFD"sv, U"\uFFFD"sv);
	encode_decode(u"\U0010AAAA"sv, U"\U0010AAAA"sv);

	auto empty_units = std::array<unit_t<utf16>, 0>{};
	auto wrong_units = std::array<char16_t, 2>{0xD811u, 0x65u};
	auto empty_codes = std::array<code_t<utf16>, 0>{};
	auto invalid_codes = std::array<char32_t, 1>{0x111111u};

	assert(utf16::encode(state, empty_codes, units).code == result_code::OK);
	assert(utf16::encode(state, empty_codes, empty_units).code == result_code::OK);
	assert(utf16::encode(state, empty_codes, units).code == result_code::OK);
	assert(utf16::encode(state, invalid_codes, units).code == result_code::INVALID_CODE_POINT);

	assert(utf16::decode(state, empty_units, codes).code == result_code::OK);
	assert(utf16::decode(state, empty_units, empty_codes).code == result_code::OK);
	assert(utf16::decode(state, wrong_units, codes).code == result_code::INVALID_ENCODING);

	assert(utf16::encode(state, std::array<char32_t, 1>{0xD8FF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf16::encode(state, std::array<char32_t, 1>{0xDCFF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf16::encode(state, std::array<char32_t, 1>{0x11FFFF}, units).code == result_code::INVALID_CODE_POINT);
}

void test_utf32() noexcept {
	auto units = std::array<unit_t<utf32>, utf32::max_units>{};
	auto codes = std::array<code_t<utf32>, utf32::max_codes>{};
	auto state = state_t<utf32>{};

	auto const encode_decode = [&](std::u32string_view test_units, std::u32string_view test_codes) {
		assert(utf32::encode(state, test_codes, units));
		assert(compare(test_units, units));
		assert(utf32::decode(state, test_units, codes));
		assert(compare(test_codes, codes));
	};

	encode_decode(U"A"sv, U"A"sv);
	encode_decode(U"\u0400"sv, U"\u0400"sv);
	encode_decode(U"\uFFFD"sv, U"\uFFFD"sv);
	encode_decode(U"\U0010AAAA"sv, U"\U0010AAAA"sv);

	auto empty_units = std::array<unit_t<utf32>, 0>{};
	auto wrong_units = std::array<char32_t, 1>{0xD811u};
	auto empty_codes = std::array<code_t<utf32>, 0>{};
	auto invalid_codes = std::array<char32_t, 1>{0x111111u};

	assert(utf32::encode(state, empty_codes, units).code == result_code::OK);
	assert(utf32::encode(state, empty_codes, empty_units).code == result_code::OK);
	assert(utf32::encode(state, empty_codes, units).code == result_code::OK);
	assert(utf32::encode(state, invalid_codes, units).code == result_code::INVALID_CODE_POINT);

	assert(utf32::decode(state, empty_units, codes).code == result_code::OK);
	assert(utf32::decode(state, empty_units, empty_codes).code == result_code::OK);
	assert(utf32::decode(state, wrong_units, codes).code == result_code::INVALID_ENCODING);

	assert(utf32::encode(state, std::array<char32_t, 1>{0xD8FF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf32::encode(state, std::array<char32_t, 1>{0xDCFF}, units).code == result_code::INVALID_CODE_POINT);
	assert(utf32::encode(state, std::array<char32_t, 1>{0x11FFFF}, units).code == result_code::INVALID_CODE_POINT);

}

int main(int argc, char const* args[]) {
	test_utf8();
	test_utf16();
	test_utf32();

	return 0;
}
