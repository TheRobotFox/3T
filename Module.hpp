#include "Memory.hpp"
#include "Special.hpp"
#include <vector>

namespace TTT {
	class Module {
		std::vector<std::string> symbols;
		std::unordered_map<std::string, SymbolExpr> name_symbol;
		long gensyms = 0;

	public:
		Env global;
		
		auto get_symbol_name(SymbolExpr s) -> std::string_view {
			if(s.id < 0) return "#GENSYM";
			if(s.id>= symbols.size()) return "#UNKNOWNSYM";
			return symbols[s.id];
		}
		auto intern(std::string s) -> SymbolExpr {
			if(name_symbol.contains(s)) return name_symbol[s];
			long id = symbols.size();
			symbols.push_back(s);
			name_symbol[s] = SymbolExpr{id};
			return SymbolExpr{id};
		}
		auto gensym() -> SymbolExpr {
			return SymbolExpr{gensyms++};
		}
		
		// source File
		Sexp body;
		Memory memory;
		std::string error;
		HashTableExpr readtable;

		auto read(std::istream text) -> bool;

		Module();
	};
}
