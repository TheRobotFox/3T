#pragma once
#include "Atom.hpp"
#include <cstddef>
#include <vector>

namespace TTT {

	struct CallEval {
		Interpreter &interp;
		Env &env;
		const Atom &self;
		Atom *out;

		template <class T>
		auto operator()(const T      &_	  ) const -> bool {*out = self; return true;};
		auto operator()(const Symbol &sym ) const -> bool;
		auto operator()(const Call   &call) const -> bool;
	};



 // 	template<class T>
 // 	concept Container = requires(T &e,	Memory &mem) {
 // e.mark_children(mem); 
 // 	};


	// struct CallMark {
	// 	Memory &mem;
	// 	template<class T>
	// 	void operator()(const T&_){};
	// 	template<Container T>
	// 	void operator()(const T&c){c.mark_children(mem);};

	// };

	class Module;
	struct Interpreter {

		Module &mod;
		std::string error;
		Interpreter(Module &mod)
		: mod(mod)
		{}

		auto eval(const Atom &x, Env &env, Atom *out) -> bool {
			
			return x.visit(CallEval{.interp=*this, .env=env, .self=x, .out = out});
		}

	};
}
