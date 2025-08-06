#include "Atom.hpp"
#include "Memory.hpp"
#include <vector>

namespace TTT {
	class Module {
		std::vector<std::string> symbols;
		std::unordered_map<std::string, SymbolId> name_symbol;
		long gensyms = 0;

	public:
		Env global;
		
		auto get_symbol_name(SymbolId s) -> std::string_view {
			if(s < 0) return "#GENSYM";
			if(s >= (long long)symbols.size()) return "#UNKNOWNSYM";
			return symbols[s];
		}
		auto intern(std::string s) -> SymbolId {
			if(name_symbol.contains(s)) return name_symbol[s];
			long id = symbols.size();
			symbols.push_back(s);
			name_symbol[s] = id;
			return id;
		}
		auto gensym() -> SymbolId {
			return gensyms--;
		}
		
		// source File

		Atom *body;
		SymbolId eof = gensym();
		Atom *readtable;
		Atom *in_port;
		Atom *out_port;
		Atom *read;

		Memory memory;

		Module();
	};
}
