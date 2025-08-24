#include "util.hpp"
#include <ankerl/unordered_dense.h>
#include <sys/types.h>

namespace TTT {
		
	struct Expression;
	struct Evaluator;
	class Atom;
	class GC;

    using Type_t = uint8_t;
	using Heap_p = void*;


    /*
     * Type
     */

    enum Type : Type_t {
        NIL,
        T,
        INT,
        REAL,
        CHAR,
		SYMBOL,
		QUOTED,
		CONS,
		TABLE,
		CLOSURE,
		MACRO,
		SPECIAL,
		ARRAY,
		FORWARD
	};

	using SymbolId = size_t;

	struct Quoted {
		SymbolId sym;
		size_t depth;
    };

	struct Cons {
		void *car, *cdr;
	};


	struct Array {
		size_t length;
		void* start;
	};
	
	using Table = ankerl::unordered_dense::map<Atom, void*>;
    using Environment = ankerl::unordered_dense::map<SymbolId, void *>;
    
	struct Closure {
		Environment env;
		std::vector<SymbolId> args;
		SymbolId rest;
		Expression *xp;
		/* Byte Code */
	};
	struct Macro {
		Environment env;
		std::vector<SymbolId> args;
		SymbolId rest;		  
		Expression *xp;
	};
	using Special = std::function<bool(Evaluator&, size_t argc, void *argv)>;


    

	/*
	 * Type Info
	 */

	using util::Assoc;
	using TypeInfo = util::_TypeInfo <Assoc<NIL, std::monostate>,
									  Assoc<T, std::monostate>,
									  Assoc<INT, long long>,
									  Assoc<CHAR, char>,
									  Assoc<REAL, double>,
									  Assoc<SYMBOL, long long>,
									  Assoc<QUOTED, Quoted>,
									  Assoc<CONS, Cons>,
									  Assoc<TABLE, Table>,
									  Assoc<CLOSURE, Closure>,
									  Assoc<MACRO, Macro>,
									  Assoc<SPECIAL, Special>,
									  Assoc<ARRAY, Array>,
									  Assoc<FORWARD, Heap_p>>;
}
