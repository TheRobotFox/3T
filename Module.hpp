#include "Expr.hpp"
#include "Memory.hpp"
#include "Special.hpp"

namespace TTT {
	struct Module {
		// source File
		std::map<SymbolExpr, std::string> symbol_name;
		std::map<std::string, SymbolExpr> name_symbol;
		size_t symbol_count;
		Expr body;
		Memory memory;
		std::string error;
		std::map<char, CallableExpr> reader_macros;
		auto read(std::istream text) -> bool;

		Module()
		{}
	};
}
