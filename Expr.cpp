#include "Expr.hpp"

using namespace TTT;

auto Environment::lookup(SymbolExpr sym) const -> Sexp*
{
	if(env.contains(sym)) return env.at(sym);
	return nullptr;
}
