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
}

#endif
