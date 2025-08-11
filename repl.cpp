#include "Interpreter.hpp"
#include "Printer.hpp"
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

	Printer printer{.out=std::cout, .mod=&mod};

	while (true) {
		std::cout << "> ";
		std::string inp;
		std::getline(std::cin, inp);
		in = InPort{.value = new std::istringstream(inp)};
		Atom result;
		if (!intp.eval(
			Call{.head = mod.f_eval,
			     .args = {Call{.head = mod.f_read, .args = {}}}},
						mod.global, &result)) {
			std::cout << "Error: " << intp.error << '\n';
		} else {
			result.visit(printer);
			std::cout << '\n';
			
		}
	}
	return 0;
}
