#include "Special.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Module.hpp"

using namespace TTT;

auto read_delimeter_(Interpreter &intp, Environment &env, Sexp *out, const InPortExpr &inp) -> bool
{
	// get closing delimeter from env
	
	CharExpr closing;
	if(auto *end = std::get_if<CharExpr>(&env.lookup(intp.mod.intern("closing"))->body)){
		closing = *end;
	}else {
		intp.error = "expected closing delimeter to be CharExpr!"; // TODO better Error
		return false;
	}

	
	// backup readtable and overwrite with EOF indicator
	
	auto &readtable = intp.mod.readtable.value;
	
	std::optional<Sexp> restore = {};
	if(readtable.contains(Sexp{closing})) restore = readtable[Sexp{closing}];

	static SymbolExpr eof = intp.mod.gensym();
	Sexp return_eof = Sexp{eof};
	readtable[Sexp{closing}] = Sexp{CallableExpr{.value = Closure{.body = &return_eof}}};

	Sexp *res = intp.mod.memory.alloc(),
		 *current = res;

	
	while() // call read -> has its own env
}    
