#include "Atom.hpp"
#include "Memory.hpp"
#include <vector>
#include <format>

namespace TTT {
	class Module {
		std::vector<std::string> symbols;
		std::unordered_map<std::string, SymbolId> name_symbol;
		long gensyms = 0;

		auto register_atom(const std::string &&name, Atom &&func) -> Atom* {
			Atom *a = allocate(std::move(func));
			global[intern(name)] = a;
			return a;
		}
		auto allocate(const Atom &&atom) -> Atom * {
			Atom *a = memory.alloc();
			*a = atom;
			return a;
		}

	public:
		Env global;
		
		auto get_symbol_name(SymbolId s) -> std::string {
			if(s < 0) return std::format("#GENSYM{}", -s);
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
		Atom *in_port;
		Atom *out_port;
		Atom *f_read;

		std::unordered_map<Atom, Atom*> readTable;

		Memory memory;

		Module();
	};
}
