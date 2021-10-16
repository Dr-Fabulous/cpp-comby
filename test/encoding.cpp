#include <cassert>
#include <cstddef>
#include <utility>
#include <functional>
#include <ranges>
#include <tuple>
#include <array>
#include <string_view>
#include <iostream>
#include "fb/comby/encoding.hpp"
#include "fb/comby/utf8.hpp"
#include "fb/comby/parser.hpp"

using namespace std::literals;
using namespace fb::comby;

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

template <typename R, typename T>
concept contiguous_range_of = std::ranges::contiguous_range<R>
			   && std::is_same_v<std::ranges::range_value_t<R>, T>;

template <encoding::encoding E>
void test_encode_decode(contiguous_range_of<encoding::unit_t<E>> auto const& test_units,
			contiguous_range_of<encoding::code_t<E>> auto const& test_codes) noexcept
{
	auto units = std::array<encoding::unit_t<E>, E::max_units>{};
	auto codes = std::array<encoding::code_t<E>, E::max_codes>{};
	auto state = encoding::state_t<E>{};

	assert(E::decode(state, test_units, codes));
	assert(compare(test_codes, codes));
	assert(E::encode(state, test_codes, units));
	assert(compare(test_units, units));
}

template <encoding::encoding E>
void test_edge_cases(contiguous_range_of<encoding::unit_t<E>> auto const& invalid_units,
		     contiguous_range_of<encoding::code_t<E>> auto const& invalid_codes,
		     contiguous_range_of<encoding::unit_t<E>> auto const& wrong_units) noexcept
{
	auto units = std::array<encoding::unit_t<E>, E::max_units>{};
	auto codes = std::array<encoding::code_t<E>, E::max_codes>{};
	auto empty_units = std::array<encoding::unit_t<E>, 0>{};
	auto empty_codes = std::array<encoding::code_t<E>, 0>{};
	auto state = encoding::state_t<E>{};

	assert(E::decode(state, empty_units, codes).code == encoding::result_code::OK); state = {};
	assert(E::decode(state, units, empty_codes).code == encoding::result_code::NOT_ENOUGH_STORAGE); state = {};
	assert(E::decode(state, empty_units, empty_codes).code == encoding::result_code::OK); state = {};
	assert(E::decode(state, invalid_units, codes).code == encoding::result_code::INVALID_CODE_POINT); state = {};
	assert(E::decode(state, wrong_units, codes).code == encoding::result_code::INVALID_ENCODING); state = {};

	assert(E::encode(state, empty_codes, units).code == encoding::result_code::OK); state = {};
	assert(E::encode(state, codes, empty_units).code == encoding::result_code::NOT_ENOUGH_STORAGE); state = {};
	assert(E::encode(state, empty_codes, empty_units).code == encoding::result_code::OK); state = {};
	assert(E::encode(state, invalid_codes, units).code == encoding::result_code::INVALID_CODE_POINT);
}

int main(int argc, char const* args[]) {
	// encode/decode characters that range 1-4 bytes
	test_encode_decode<encoding::utf8>(u8"A"sv, U"A"sv);
	test_encode_decode<encoding::utf8>(u8"\u0400"sv, U"\u0400"sv);
	test_encode_decode<encoding::utf8>(u8"\uFFFD"sv, U"\uFFFD"sv);
	test_encode_decode<encoding::utf8>(u8"\U0010AAAA"sv, U"\U0010AAAA"sv);

	// units and codes that go beyond 0x10FFFF and an invalid unit sequence
	test_edge_cases<encoding::utf8>(std::array<char8_t, 4>{0b11110111, 0b10111111, 0b10111111, 0b10111111},
					std::array<char32_t, 1>{0x111111},
					std::array<char8_t, 2>{0b11000011, 0});

	// surrogate values
	{
		auto surrogate_units_1 = std::array<char8_t, 3>{0b11101101, 0b10100000, 0b10000001};
		auto surrogate_units_2 = std::array<char8_t, 3>{0b11101101, 0b10110000, 0b10000001};
		auto surrogate_codes_1 = std::array<char32_t, 1>{0xD801};
		auto surrogate_codes_2 = std::array<char32_t, 1>{0xDC01};
		auto units = std::array<encoding::unit_t<encoding::utf8>, encoding::utf8::max_units>{};
		auto codes = std::array<encoding::code_t<encoding::utf8>, encoding::utf8::max_codes>{};
		auto state = encoding::utf8::state_type{};

		assert(encoding::utf8::decode(state, surrogate_units_1, codes).code == encoding::result_code::INVALID_CODE_POINT);
		assert(encoding::utf8::decode(state, surrogate_units_2, codes).code == encoding::result_code::INVALID_CODE_POINT);
		assert(encoding::utf8::encode(state, surrogate_codes_1, units).code == encoding::result_code::INVALID_CODE_POINT);
		assert(encoding::utf8::encode(state, surrogate_codes_2, units).code == encoding::result_code::INVALID_CODE_POINT);
	}

	return 0;
}
