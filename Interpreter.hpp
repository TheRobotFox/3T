#include "Expr.hpp"
#include <expected>


namespace TTT {

	struct Module;
	
	class Interpreter {
		auto eval(const Sexp &x, Environment &env, Sexp *out) -> bool;
		auto eval_closure( Closure &cl,  Environment& env, ConsExpr args, Sexp *out) -> bool;
		auto eval_special( Special &fn, Environment& env, ConsExpr args, Sexp *out) -> bool;
	public:

		Module &mod;
		std::string error;
		Interpreter(Module &mod)
		: mod(mod)
		{}

		auto evaluate(Sexp x, Environment &env) -> std::expected<Sexp, std::string>;

	};
}
