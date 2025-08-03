#include "Interpreter.hpp"
#include <cstddef>
#include <format>
#include <utility>
#include <variant>
#include <vector>

using namespace TTT;


auto CallEval::operator()(Symbol &sym) const -> Atom* {
	if (env.contains(sym.id))
		return env.at(sym.id);
	if (interp.mod.global.contains(sym.id))
		return interp.mod.global.at(sym.id);
	interp.error = std::format("Symbol %s is not in scope!", interp.mod.get_symbol_name(sym.id));
	return nullptr;
}

struct Cons_iterator {
	Atom *current;
	auto next() -> Atom * {
		if (auto *cons = std::get_if<Cons>(current)) {
			current = cons->cdr;
			return cons->car;
		}
		return nullptr;
	}
	auto count() -> size_t {
		size_t length = 0;
		while (next() != nullptr)
			length++;
		return length;
	}
};

struct CallProc {
	Interpreter &interp;
	Env &env;
	Cons_iterator args;

	auto operator()(Closure &c) -> Atom * {
		size_t args_expected = c.args.size(), args_recieved = 0;

		for (; args_recieved < args_expected; args_recieved++) {
			if(auto *arg = args.next()) {
				if (auto *eval = interp.eval(*arg, env)) {
					c.env[ c.args[args_recieved] ] = eval;
				} else
					return nullptr;
			} else goto arg_count_error;
		}
		if (c.rest != 0) {
			Atom *res = interp.mod.memory.alloc(), *end = res;
			while (auto *arg = args.next()) {
				if (auto *eval = interp.eval(*arg, env)) {
					Atom *next = interp.mod.memory.alloc();					
					*end = Cons{.car = eval, .cdr = next};
					end = next;
				} else
					return nullptr;
			}
			if (auto *eval = interp.eval(*args.current, env)) {
				*end = *eval;
			} else
				return nullptr;
			c.env[c.rest] = args.current;
		} else if (!std::holds_alternative<nil>(*args.current)) {
			args_recieved += args.count();
			goto arg_count_error;
			
		}

		return interp.eval(*c.body, c.env);
		
 arg_count_error:
		interp.error = std::format("Lambda expected %d args but got %d", args_expected, args_recieved);
		return nullptr;
	}

		auto operator()(Macro &c) -> Atom * {
		
		}
		auto operator()(Special &c) -> Atom * {}
	
		template<class T> auto operator()(T &c) -> Atom * {
			interp.error = std::format("Atom is not valid Function!");
			return nullptr;
		}
	};

	auto CallEval::operator()(Call &call) const -> Atom * {
		Atom *fn = interp.eval(*call.head, env);

		fn->visit(CallProc{.interp=interp, .env=env, .args=call.args});
	}

	auto Interpreter::eval_special( Special &fn, Environment &env, ConsExpr args, Sexp *out) -> bool
	{
		return fn.func(*this, args, env, out);
	}	 


	auto Interpreter::eval(const Sexp &x, Environment &env, Sexp *out) -> bool
	{
		if(const auto* sym = std::get_if<SymbolExpr>(&x.body)){
		
			if(auto *s = env.lookup(*sym)){
				*out = *s;
				return true;
			}
			error = std::format("Could not find Symbol %s in Environment!", mod.symbol_names[*sym]);
			return false;
		}
		if(const auto* cons = std::get_if<ConsExpr>(&x.body)){

			Sexp eval_fn;
			if(!eval(*cons->car, env, &eval_fn)) return false;

			if(auto *callable = std::get_if<CallableExpr>(&eval_fn.body)){
				if( auto *cl = std::get_if<Closure>(&callable->value))
					return eval_closure(*cl, env, *cons, out);
				if( auto *mc = std::get_if<Macro>(&callable->value))
					return false;
				if( auto *sp = std::get_if<Special>(&callable->value))
					return eval_special(*sp, env, *cons, out);
			}
			error = std::format("Form is not a callable!");
			return false;
		}
		*out = x;
		return true;
	}	 

