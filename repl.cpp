#include "Atom.hpp"
#include "Interpreter.hpp"
#include <iostream>
#include <sstream>

int main(void) {

	Module mod;

	Atom in;
	mod.in_port = &in;
	Interpreter intp(mod);
	
	while (true) {
		std::string inp;
		std::cin >> inp;
		std::istringstream ss(inp);
		in = InPort{.value = &ss};
		Atom result;
		intp.eval(Call{.head = mod.f_read}, mod.global, &result);
		std::cout << result;
	}
	return 0;
}
