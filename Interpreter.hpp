#include "Module.hpp"
#include <expected>


namespace TTT {
	class Interpreter {
		Module &mod;
		auto eval(const Sexp &x, Environment &env, Sexp *out) -> bool;
		auto eval_closure( Closure &cl,  Environment& env, ConsExpr args, Sexp *out) -> bool;
		auto eval_special( Special &fn, Environment& env, ConsExpr args, Sexp *out) -> bool;
	public:

		std::string error;
		Interpreter(Module &mod)
		: mod(mod)
		{}

		auto evaluate(Sexp x, Environment &env) -> std::expected<Sexp, std::string>;

	};
}
