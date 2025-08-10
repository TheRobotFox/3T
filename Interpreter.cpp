#include "Interpreter.hpp"
#include "Module.hpp"
#include "Special.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <utility>
#include <vector>

using namespace TTT;


auto CallEval::operator()(const Symbol &sym) const -> bool {
	if (env.contains(sym.id)){
		*out = *env.at(sym.id);
		return true;
	}
	if (interp.mod.global.contains(sym.id)){
		*out = *interp.mod.global.at(sym.id);
		return true;
	}
	interp.error = std::format("Symbol %s is not in scope!", interp.mod.get_symbol_name(sym.id));
	return false;
}

struct CallProc {
	Interpreter &interp;
	Env &env;
	const std::vector<Atom> &args;
	Atom *out;
	
	template<class T> auto operator()(T &c) -> bool {
		interp.error = std::format("Atom is not valid Function!");
		return false;
	}
	auto operator()(Closure &c) -> bool {
		if (args.size() < c.args.size() ||
			(args.size() > c.args.size() && c.rest == 0)) {
			interp.error = argument_error(c.args.size(), args.size());
			return false;
		}

		for (size_t i = 0; i < args.size(); i++) {
			Atom *arg = interp.mod.memory.alloc();
			if (!interp.eval(args[i], env, arg)) return false;
			c.env[ c.args[i] ] = arg;
		}
		return interp.eval(*c.body, c.env, out);
	}
	auto operator()(Macro &c) -> bool {
		interp.error = "Macros Not Implemented!";
		return false;
	}
	auto operator()(Special &c) -> bool {
		return c.func(interp, env, args, out);
	}
};

auto CallEval::operator()(const Call &call) const -> bool {
	Atom *fn;
	if (!interp.eval(*call.head, env, fn))
		return false;
	
	return fn->visit(CallProc{.interp=interp, .env=env, .args= call.args, .out = out});
}
