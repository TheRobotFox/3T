#include "Special.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Module.hpp"
#include <variant>

using namespace TTT;

auto is_cons(Interpreter &interpreter, ConsExpr sexp, Environment &env,
			 Sexp *out) -> bool {
	if (const auto *x = std::get_if<ConsExpr>(&sexp.cdr->body)) {
		if (std::holds_alternative<ConsExpr>(x->car->body)) {
			*out = 
	  }
	}
}

auto read_delimeter_(Interpreter &intp, Environment &env, Sexp *out,
					 const InPortExpr &inp) -> bool {
	// get closing delimeter from env

	CharExpr closing;
	if (auto *end = std::get_if<CharExpr>(
										  &env.lookup(intp.mod.intern("closing"))->body)) {
		closing = *end;
	} else {
		intp.error =
			"expected closing delimeter to be CharExpr!"; // TODO better Error
		return false;
	}

	// backup readtable and overwrite with EOF indicator

	auto &readtable = intp.mod.readtable.value;

	std::optional<Sexp> restore = {};
	if (readtable.contains(Sexp{closing}))
		restore = readtable[Sexp{closing}];

	static Sexp eof	   = Sexp{intp.mod.gensym()};
	readtable[Sexp{closing}] = Sexp{CallableExpr{.value = Closure{.body = &eof}}};

	// read until eof indicator

	Sexp *res = intp.mod.memory.alloc(), *current = res;
	Sexp *read = intp.mod.global.lookup(intp.mod.intern("read"));

	while (true) {
		if (intp.eval(*read, env, current))
			return false;
		if (*current == eof)
			break;

		// push new cons
		res		   = intp.mod.memory.alloc();
		Sexp *next = intp.mod.memory.alloc();
		*res	   = Sexp{ConsExpr{.car = current, .cdr = next}};
		current	   = next;
	}
	
	*current = Sexp{Null{}};

	*out = *res;
	return true;
}
