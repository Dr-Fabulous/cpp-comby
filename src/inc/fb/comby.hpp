#ifndef FB_COMBY_HPP
#define FB_COMBY_HPP
#include <type_traits>
#include <concepts>
#include <iterator>
#include <tuple>
#include "fb/result.hpp"

namespace fb::comby {
	namespace detail {
		template <typename T, template <typename...> typename U>
		struct is_template_of : public std::false_type {};

		template <template <typename...> typename U, typename... Us>
		struct is_template_of<U<Us...>, U> : public std::true_type {};

		template <typename T, template <typename...> typename U>
		concept template_of = is_template_of<T, U>::value;
	}

	template <typename T, std::input_iterator It>
	struct partial_result {
		using value_type = T;
		using iterator = It;

		value_type value;
		iterator pos;
		iterator end;
	};

	template <typename V, typename E, std::input_iterator It>
	using result = fb::result<partial_result<std::decay_t<V>, It>,
	      			  partial_result<std::decay_t<E>, It>>;

	template <typename E, typename It>
	constexpr auto make_error(E&& e, It pos, It end) {
		return fb::make_error<partial_result<std::decay_t<E>, It>>(std::forward<E&&>(e), pos, end);
	}
}

#endif
