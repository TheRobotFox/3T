#pragma once
#include "util.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace TTT {
		
	struct Expression;
	struct Evaluator;
	class Atom;
	class GC;

    using Type_t = uint_fast16_t;
	using Heap_p = void*;


    /*
     * Type
     */

    enum Type : Type_t {
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
        FORWARD,
        NIL,
        T
    };

    template <class T>
    concept Container = requires(T &e) {
			e.markChildren();
    };

	using SymbolId = size_t;

	struct Quoted {
		SymbolId sym;
		size_t depth;
    };

	struct Cons {
        void *car, *cdr;
        
        void markChildren() const;
	};


	struct Array {
		size_t length;
        std::byte *start;
        
        void markChildren() const;        
	};

    struct Table {
		std::unordered_map<Heap_p, Heap_p> value;
		
        void markChildren() const;
	};
    using Environment = std::unordered_map<SymbolId, Heap_p>;
    
	struct Closure {
		Environment env;
		std::vector<SymbolId> args;
		SymbolId rest;
		Expression *xp;
        /* Byte Code */
        
        void markChildren() const;
    };
    
    using Special =
            std::function<bool(Evaluator &, size_t argc, void *argv)>;

    struct Forward {
		Heap_p ref;
		void markChildren() const;
    };

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
									  Assoc<MACRO, Closure>,
									  Assoc<SPECIAL, Special>,
									  Assoc<ARRAY, Array>,
									  Assoc<FORWARD, Forward>>;

    struct Mark {
        void *cell;
		template <size_t id> void operator()() {
			using T = TypeInfo::Type<id>;
			if constexpr (Container<T>)
				reinterpret_cast<T*>(cell)->markChildren();
		}
	};
}
