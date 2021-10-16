#ifndef FB_CPP_COMBY_ENCODING_HPP
#define FB_CPP_COMBY_ENCODING_HPP
#include <cstddef>
#include <type_traits>
#include <ranges>
#include <span>
#include <string_view>
#include "fb/tag_invoke.hpp"
#include "fb/comby/concepts.hpp"

namespace fb::comby::encoding {
	template <typename E> using unit_t = typename E::unit_type;
	template <typename E> using code_t = typename E::code_type;
	template <typename E> using state_t = typename E::state_type;

	enum class result_code {
		OK,
		NOT_ENOUGH_STORAGE,
		INVALID_CODE_POINT,
		INVALID_ENCODING
	};

	template <typename E>
	struct encode_result {
		result_code code;
		std::span<code_t<E> const> src;
		std::span<unit_t<E>> dst;

		constexpr operator bool() const noexcept {
			return code == result_code::OK;
		}
	};

	template <typename E>
	struct decode_result {
		result_code code;
		std::span<unit_t<E> const> src;
		std::span<code_t<E>> dst;

		constexpr operator bool() const noexcept {
			return code == result_code::OK;
		}
	};

	template <typename E>
	concept encoding = requires() {
		typename unit_t<E>;
		typename code_t<E>;
		typename state_t<E>;

		{E::max_units} -> concepts::same_as<std::size_t const&>;
		{E::max_codes} -> concepts::same_as<std::size_t const&>;

		requires requires(
			state_t<E>& state,
			std::array<unit_t<E>, E::max_units>&& units,
			std::array<code_t<E>, E::max_codes>&& codes
		) {
			{E::encode(state, codes, units)} -> concepts::same_as<encode_result<E>>;
			{E::decode(state, units, codes)} -> concepts::same_as<decode_result<E>>;
		};
	};
}

#endif
