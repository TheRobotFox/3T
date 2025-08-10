#include "Atom.hpp"
#include "Interpreter.hpp"
#include "Module.hpp"
#include <iostream>
#include <sstream>
#include <string>

std::string print(Atom &a) {
	std::ostringstream out;
	out << a;
	return out.str();
}

int main(void) {

	Module mod;

	Atom in;
	mod.in_port = &in;
	Interpreter intp(mod);

	while (true) {
		std::cout << "> ";
		std::string inp;
		std::getline(std::cin, inp);
		in = InPort{.value = new std::istringstream(inp)};
		Atom result;
		if (!intp.eval(Call{.head = mod.f_read, .args = {}}, mod.global,
						&result)) {
			std::cout << "Error: " << intp.error << '\n';
		}else std::cout << result << '\n';
	}
	return 0;
}
