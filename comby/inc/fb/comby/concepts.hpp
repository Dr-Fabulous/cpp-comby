#ifndef FB_COMBY_CONCEPTS_HPP
#define FB_COMBY_CONCEPTS_HPP
#include <type_traits>
#include <iterator>
#include <ranges>

namespace fb::comby::concepts {
	template <typename T, typename U>
	concept same_as = std::is_same_v<T, U>;

	namespace detail {
		template <typename T, template <typename...> typename U>
		struct is_template_of : std::false_type {};

		template <template <typename...> typename U, typename... Us>
		struct is_template_of<U<Us...>, U> : std::true_type {};
	}

	template <typename T, template <typename...> typename U>
	concept template_of = detail::is_template_of<T, U>::value;

	namespace detail {
		template <typename T, typename U, typename... Us>
		struct is_type_in_list : std::conditional_t<
			std::is_same_v<T, U>,
			std::true_type,
			typename is_type_in_list<T, Us...>::type
		> {};

		template <typename T, typename U>
		struct is_type_in_list<T, U> : std::bool_constant<std::is_same_v<T, U>> {};
	}

	template <typename T, typename U, typename... Us>
	concept type_in_list = detail::is_type_in_list<T, U, Us...>::value;

	template <typename I>
	concept integral = type_in_list<std::remove_cvref_t<I>,
					char, unsigned char,
					short, unsigned short,
					int, unsigned int,
					long, unsigned long,
					long long, unsigned long long>;
}

#endif
