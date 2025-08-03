#include "Atom.hpp"
#include <expected>


namespace TTT {

	template<class T>
	concept Evaluating = requires(T &e,	Interpreter &interp, Env &env) {
 {e.eval(interp, env) } -> std::same_as<Atom*>;
	};

	struct CallEval {
		Interpreter &interp;
		Env &env;
		Atom &self;

		template <class T>		auto operator()(T &_) const -> Atom* {return &self;};
		auto operator()(Symbol &sym) const -> Atom*;
		auto operator()(Call &call) const -> Atom*;
	};



	template<class T>
	concept Container = requires(T &e,	Memory &mem) {
 e.mark_children(mem); 
	};


	struct CallMark {
		Memory &mem;
		template<class T>
		void operator()(const T&_){};
		template<Container T>
		void operator()(const T&c){c.mark_children(mem);};

	};


	struct Interpreter {
		
		
		Module &mod;
		std::string error;
		Interpreter(Module &mod)
		: mod(mod)
		{}

		auto eval(Atom &x, Env &env) -> Atom * {
			return x.visit(CallEval{.interp=*this, .env=env, .self=x});
		}

	};
}
