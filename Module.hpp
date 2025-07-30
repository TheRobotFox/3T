#include "Memory.hpp"

namespace TTT {
	struct Module {
		// source File
		std::map<SymbolExpr, std::string> symbol_names;
		Expr body;
		Memory memory;
	};
}
