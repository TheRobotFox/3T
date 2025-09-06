#pragma once
#include "Atom.hpp"
#include "Types.hpp"
#include <cstddef>
#include <vector>

namespace TTT {

	class Module;
	struct Interpreter {

		Module &mod;
		std::string error;
		Interpreter(Module &mod)
		: mod(mod)
		{}

		auto eval() -> Heap_p {
			
			return x.visit(CallEval{.interp=*this, .env=env, .self=x, .out = out});
		}

	};
}
