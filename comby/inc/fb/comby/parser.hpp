#ifndef FB_COMBY_PARSER_TRAITS_HPP
#define FB_COMBY_PARSER_TRAITS_HPP
#include <type_traits>
#include <optional>
#include <variant>
#include <functional>
#include "fb/tag_invoke.hpp"

namespace fb::comby {
	namespace detail {
		template <typename T, template <typename...> typename U>
		struct is_template_of : std::false_type {};

		template <template <typename...> typename U, typename... Us>
		struct is_template_of<U<Us...>, U> : std::true_type {};

		template <typename T, template <typename...> typename U>
		concept template_of = is_template_of<T, U>::value;
	}

	template <typename P> using parser_char_t = typename P::char_type;
	template <typename P> using parser_value_t = typename P::value_type;
	template <typename P> using parser_error_t = typename P::error_type;

	inline constexpr struct parse_t {
		template <typename P, typename It, typename S>
		constexpr auto operator()(P&& p, It pos, S end) const noexcept(fb::is_nothrow_tag_invocable_v<parse_t, P&&, It, S>)
									    -> fb::tag_invoke_result_t<parse_t, P&&, It, S>
		{
			return fb::tag_invoke(*this, std::forward<P>(p), pos, end);
		}
	} parse = {};

	inline constexpr struct default_result_t {} default_result;

	template <typename CharT, typename V, typename E, typename It, typename S>
	class parser_result : public std::variant<E, V> {
	public:
		using char_type = CharT;
		using value_type = V;
		using error_type = E;
		using iterator = It;
		using sentinel = S;
		using variant_type = std::variant<error_type, value_type>;
		using variant_type::variant;

	private:
		iterator m_pos;
		sentinel m_end;

	public:
		explicit constexpr parser_result(default_result_t, iterator pos, sentinel end) :
			m_pos{pos},
			m_end{end}
		{}

		constexpr void set_error(error_type const& err, iterator pos, sentinel end) {
			emplace<0>(err);
			m_pos = pos;
			m_end = end;
		}

		constexpr void set_error(error_type&& err, iterator pos, sentinel end) {
			emplace<0>(std::move(err));
			m_pos = pos;
			m_end = end;
		}

		constexpr void set_value(value_type const& v, iterator pos, sentinel end) {
			emplace<1>(v);
			m_pos = pos;
			m_end = end;
		}

		constexpr void set_value(value_type&& v, iterator pos, sentinel end) {
			emplace<1>(std::move(v));
			m_pos = pos;
			m_end = end;
		}

		constexpr error_type& error() {
			return std::get<0>(*this);
		}

		constexpr error_type const& error() const {
			return std::get<0>(*this);
		}

		constexpr value_type& value() {
			return std::get<1>(*this);
		}

		constexpr value_type const& value() const {
			return std::get<1>(*this);
		}

		constexpr iterator pos() const noexcept {
			return m_pos;
		}

		constexpr sentinel end() const noexcept {
			return m_end;
		}
	};

	template <typename P>
	concept parser = requires(P&& p) {
		typename parser_char_t<P>;
		typename parser_value_t<P>;
		typename parser_error_t<P>;

		{parse(p,
		       std::declval<parser_char_t<P> const*>(),
		       std::declval<parser_char_t<P> const*>())} -> detail::template_of<parser_result>;
	};

	template <typename CharT, typename V, typename E, typename F>
	class wrapped_parser {
	public:
		using char_type = CharT;
		using value_type = V;
		using error_type = E;
		using invocable_type = F;

	private:
		invocable_type m_f;

	public:
		constexpr wrapped_parser() = delete;
		constexpr wrapped_parser(wrapped_parser const&) = default;
		constexpr wrapped_parser(wrapped_parser&&) = default;

		explicit constexpr wrapped_parser(invocable_type const& f) :
			m_f{f}
		{}

		explicit constexpr wrapped_parser(invocable_type&& f) :
			m_f{std::move(f)}
		{}

		constexpr wrapped_parser& operator=(wrapped_parser const&) = default;
		constexpr wrapped_parser& operator=(wrapped_parser&&) = default;

		template <typename It, typename S>
		friend constexpr parser_result<char_type, value_type, error_type, It, S> tag_invoke(fb::tag_t<parse>, wrapped_parser& p, It pos, S end) {
			auto r = parser_result<char_type, value_type, error_type, It, S>{default_result, pos, end};
			std::invoke(p.m_f, pos, end, r);
			return r;
		}
	};

	template <typename CharT, typename V, typename E, typename F>
	constexpr wrapped_parser<CharT, V, E, F> as_parser(F const& f) {
		return {f};
	}

	template <typename CharT, typename V, typename E, typename F>
	constexpr wrapped_parser<CharT, V, E, F> as_parser(F&& f) {
		return {std::move(f)};
	}
}

#endif
