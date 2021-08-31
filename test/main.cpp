#include "fb/comby.hpp"
#include "test.hpp"

constinit auto tests = std::tuple{
	[](){

	}
};

int main(int argc, char const* args[]) {
	return test::run(tests);
}
