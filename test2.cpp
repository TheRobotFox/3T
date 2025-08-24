#include "Atom.hpp"
#include <cstddef>
#include <memory>
#include <print>

using namespace TTT;

int main() {

	std::print("{}", util::lcm<4, 2, 8, 16, 24>());
}
