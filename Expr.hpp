#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <string>

namespace TTT {

	struct Cons;

	struct SymbolExpr	{size_t      id;	  };
	struct NumberExpr	{double value;};
	struct IntegerExpr	{int		value;};
	struct StringExpr	{std::string		value;};
	struct CharExpr		{char	value;};
	struct CallableExpr;
	struct Sexp;
	struct ConsExpr		{Sexp  *car, *cdr;};
	struct Null {};

	using Expr = std::variant<SymbolExpr,
							  NumberExpr,
							  Null,
							  IntegerExpr,
							  StringExpr,
							  CharExpr,
							  CallableExpr,
							  ConsExpr>;

	struct Environment {
		using Env = std::map<SymbolExpr, Sexp*>;
		Env env;
		auto lookup(SymbolExpr sym) const -> Sexp*;
		void bind(SymbolExpr sym, Sexp* value){env[sym]=value;}
	};


	struct Macro {
		Environment env;
		std::vector<SymbolExpr> args;
        std::optional<SymbolExpr> rest;
        Sexp *body;
	};
	
	struct Closure {
		Environment env;
		std::vector<SymbolExpr> args;
        std::optional<SymbolExpr> rest;
		Sexp *body;
		// TODO byte code

	};

	struct Special {
		std::function<bool(ConsExpr, Environment&, Sexp*, std::string*)> func;
	};
	
	struct CallableExpr {
		std::variant<Closure, Macro, Special> value;
	};

	struct Sexp {
		Expr body;
	};
}
