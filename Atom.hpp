#include "util.hpp"
#include <ankerl/unordered_dense.h>
#include <bits/cxxabi_init_exception.h>
#include <cassert>
#include <cstddef>
#include <unordered_map>
#include "GC.hpp"

namespace TTT {

	class Atom;
	struct Expression;
    struct Evaluator;
    class Header;

	/*
	 * Types
	 */

    enum Type : uint8_t {
      NIL,
      T,
      INT,
      REAL,
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

	using SymbolId = long long;


	struct Quoted {
		SymbolId sym;
		long long depth;
	};

	struct Cons {
		Atom *car, *cdr;
    };


    struct Array;
    
	using Table = ankerl::unordered_dense::map<Atom, Atom *>;
	using Environment = std::unordered_map<SymbolId, Atom*>;
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
	using Special = std::function<bool(Evaluator&, size_t argc, Atom *argv)>;


	/*
	 * Type Info
	 */

	using util::Assoc;
	using TypeInfo = util::_TypeInfo <Assoc<NIL, std::monostate>,
									  Assoc<T, std::monostate>,
									  Assoc<INT, long long>,
									  Assoc<REAL, double>,
									  Assoc<SYMBOL, long long>,
									  Assoc<QUOTED, Quoted>,
									  Assoc<CONS, Cons>,
									  Assoc<TABLE, Table>,
									  Assoc<CLOSURE, Closure>,
									  Assoc<MACRO, Macro>,
									  Assoc<SPECIAL, Special>,
									  Assoc<ARRAY, Array>,
									  Assoc<FORWARD, Atom*>>;
    class Header {
    protected:
		size_t data;
		constexpr void set(size_t bit) { data |= bit;}
        constexpr void unset(size_t bit) { data &= ~bit; }
        constexpr void setType(Type_t type) { data = (data & (~typeMask)) | type; }
        

    private:
		constexpr static const size_t typeBits = util::repBits(TypeInfo::count);
		constexpr static const size_t typeMask = (1 << typeBits) - 1;
                constexpr static const size_t sizeMask =
                    ((1LLU << (sizeof(data) * 8 - (typeBits + 3))) - 1)
                    << (typeBits + 3);

		constexpr static auto makeSize(size_t size) -> size_t {
            assert(size < 1 << (typeBits + 3) && "Max Atom size exeeced!");
            return size << (typeBits + 3);
        }

        
        struct SizeOf {
			template<size_t id>
			static constexpr auto operator()()-> size_t {return sizeof(TypeInfo::Type<id>);}	
        };

	public:
		constexpr static const size_t markBit	  = 1 << (typeBits + 1);
		constexpr static const size_t processedBit = 1 << (typeBits + 2);

		auto getType() const -> Type_t { return data & typeMask; }
		auto getSize() const -> Type_t {
			return (data & sizeMask) >> (typeBits + 3);
		}
		constexpr auto check(size_t bit) const -> bool {
			return (data & bit) != 0;
		}

		Header(Type_t type) : data(type | markBit) {
			assert(type != ARRAY &&
				   "Using basic constructor for ARRAY does not work!");
			data |= makeSize(TypeInfo::btable<SizeOf>[type]);
		}
		Header(Header subtype, size_t length);

	};
	class Atom : public Header {
		void *data[0];

	
		// Allocate Atom on heap, move object return address
		struct Move {
			Atom &self;
			use_gc &memory;
			template <size_t id> Atom* operator()() {
				Atom *target = memory.alloc(self.Header::data);
				target->get<id>() = std::move(self.get<id>());
				return target;
			}
		};

	public:
        constexpr Atom(Header &&h) : Header(h) {}
        

		auto ptr() -> Atom* {
			if (getType() == FORWARD) {
				auto *res = reinterpret_cast<Atom *>(data);
				assert(FORWARD != res->getType() && "WARNING: Nested Forwards!");
				return res;
			}
			return this;
			
		};
		template <size_t id> auto get() -> TypeInfo::Type<id> & {
			Atom *p = ptr();
			assert(id == p->getType() &&
				   "Atom requested wrong type!");
			
			return *reinterpret_cast<TypeInfo::Type<id>*>(p->data);
		}

		auto moveHeap(use_gc &memory) {
			if(getType() != FORWARD){
				*reinterpret_cast<Atom **>(&data) =
					TypeInfo::vtable<Move>[getType()](
													  {.self = *this, .memory = memory});
                setType(FORWARD);
			}
		}
		
		// used to get pointer to next Argument on Stack
		auto next() -> Atom* {
			return reinterpret_cast<Atom *>(
									   reinterpret_cast<std::byte *>(&data) +
									   getSize());
		}
    };



    struct Array {
		Header header;
		const size_t length;
        void *data[0];
    };

};
