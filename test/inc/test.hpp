#ifndef FB_CPP_RESULT_TEST_HPP
#define FB_CPP_RESULT_TEST_HPP
#include <concepts>
#include <utility>
#include <functional>
#include <tuple>
#include <source_location>
#include <iostream>

namespace test {
	struct failed_assertion {
		std::source_location loc;
	};

	void assert(bool b, std::source_location loc = std::source_location::current()) {
		if (!b) {
			throw failed_assertion{loc};
		}
	}

	template <std::invocable<>... Fs>
	int run(std::tuple<Fs...> const& t) {
		try {
			std::apply([](auto&&... fs) {
				(std::invoke(fs), ...);
			}, t);
		} catch(failed_assertion& a) {
			std::cout << "\033[1;31mERROR\033[0m: failure at "
				  << a.loc.file_name()
				  << ":"
				  << a.loc.line()
				  << " in "
				  << a.loc.function_name()
				  <<  "\n";

			return 1;
		} catch(...) {
			std::cerr << "\033[1;31mERROR\033[0m: unknown exception\n";
			return 2;
		}

		return 0;
	}
}

#endif
