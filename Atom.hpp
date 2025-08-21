#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ankerl/unordered_dense.h>
#include <memory>
#include <type_traits>
#include "util.hpp"

namespace TTT {

    class Evaluator;


    
	/*
	 * Atom Types
	 */


	enum Type : Type_t {
		NIL,
		T,
		INT,
		REAL,
		STRING,
		SYMBOL,
		QUOTED,
		CONS,
		TABLE,
		CLOSURE,
		MACRO,
		SPECIAL,
		ARRAY
	};

	using Nil = struct{};
	using t = struct {};

    struct Array;

    struct Quoted {
		long long sym;
		long long depth;
    };
    struct Cons;
    struct Closure;
    using Macro = Closure;
    struct Special;
    struct Table;

    template <class... Types> class AtomData;
    
    using util::AtomType;
    using Atom = AtomData < AtomType<NIL, Nil>, AtomType<T, t>,
						   AtomType<INT, long long>, AtomType<REAL, double>,
						   AtomType<STRING, std::string>, AtomType<SYMBOL, long long>,
						   AtomType<QUOTED, Quoted>, AtomType<CONS, Cons>,
						   AtomType<TABLE, Table>,
						   AtomType<CLOSURE, Closure>, AtomType<MACRO, Macro>,
						   AtomType<SPECIAL, Special>,
						   AtomType<ARRAY, Array>>;



	

	// Compact Header tries uses Bit-Ops to fit fields into single Byte
	template<size_t TypeCount>
	class CompactHeader {
        size_t data{markBit};

        constexpr static const size_t typeBits = util::repBits(TypeCount);

        constexpr static const size_t typeMask =
            (1 << typeBits) - 1;
        constexpr static const size_t markBit =
            1 << (typeBits + 1);
        constexpr static const size_t processedBit =
            1 << (typeBits + 2);
        constexpr static const size_t refBit =
            1 << (typeBits + 3);            
        
	public:
		
		constexpr auto getType() const -> Type_t	{return		 data &	 typeMask	  ;}
        constexpr void setType(Type_t t)    		{data = (data & ~typeMask) | t;}


		constexpr auto isRef() const	 -> Bool_t {return data & refBit;}
		constexpr void setRef() 		 	  	   {data |=  refBit;}
		constexpr void unsetRef() 		 	  	   {data &= ~refBit;}
        
		constexpr auto isMarked() const -> Bool_t { return data & markBit; }
		constexpr void mark() {data|= markBit;}
		constexpr void unmark() {data &= ~markBit;}

        constexpr auto isProcessed() const -> Bool_t {return data & processedBit;  }
		constexpr void setProcessed() 				 {data |= 	processedBit; }
		constexpr void unsetProcessed() 			 {data &=  ~processedBit; }

		constexpr CompactHeader(Type_t type, bool ref = false) {
			setType(type);
			if (ref) setRef();
		}
	};


	
    
	// Select Header to use
	template<size_t TypeCount>
	using use_header = CompactHeader<TypeCount>;


    
	// Variant-esque Implementation of Generic Box
	template <class... Types>
	class AtomData : private Types..., public use_header<sizeof...(Types)> {
        static_assert(util::_check_ids_well_ordered<Types...>(),
                        "TypeIds must form Interval 0..n!");
	public:
        using header = use_header<sizeof...(Types)>;
        template <size_t id>
        using Type = Types...[id];

		template <class E, size_t id = 0>
		static constexpr auto type_id() -> size_t {
			if constexpr (id>=sizeof...(Types)) return id;
			else if constexpr (std::is_same_v<typename Types...[id] ::Type, E>)
				return id;
			else return type_id<E, id+1>();
		}

		
		// Impl visit
		template <class Fn> auto visit(Fn &&fn) {
			using Result = decltype(fn((Types...[0]{})));
			static constexpr std::array<Result (*)(Fn&&, AtomData<Types...> &), sizeof...(Types)> vtable = {&invoke<Fn, Types::Id> ...};
			return vtable[header::getType()](std::forward<Fn>(fn), *this);
        }

		template <template <size_t> class Visitor, class... Args> auto visitId(Args &&...args) {
			using Result = decltype(Visitor<0>::operator()(args...));

			static constexpr std::array<Result (*)(Args...), sizeof...(Types)> vtable = {&Visitor<Types::Id>::operator() ...};
			return vtable[header::getType()](args...);
		}

        template <size_t id>
		auto get_unchecked() -> util::index_pack<id, Types...>::Type::Type & { // G++ Struggels with mangleing type-parameter Pack indexing
			static_assert(id<sizeof...(Types), "TypeId out of Range!");
			if (this->isRef())
				return *reinterpret_cast<Types...[id]::Type *>(data[0]);
			return *reinterpret_cast<Types...[id]::Type*>(&data);
		}

     	template <size_t id>
		auto get() -> Types... [id] ::Type* {
			if (header::getType() != id)
				return nullptr;
			return &get_unchecked<id>();
		}
		
		template <size_t id>
		auto size_of() -> size_t {
			assert(header::getType() != id && "AtomData holds different Type, could not obtain size!");
			return Types...[id]::size_of(get_unchecked<id>())+sizeof(*this);
		}

	private:

		void* data[0];


		// dispatcher for VTable
		template <class Fn, size_t id>
		static decltype(auto) invoke(Fn &&fn, AtomData &ad) {
			// if constexpr (std::is_same_v<typename Types...[id] ::Type, void>)
			//	exit(-1);
			// else
			return std::invoke(fn, ad.get_unchecked<id>());
        }
	public:
          template <size_t id>
		  AtomData(Types...[id] ::Type &&data) : header(id) {
			  using T = Types...[id] ::Type;
			  static_assert(id < sizeof...(Types),
								"Type is not part of Atom, failed to "
								"retrieve TypeId!");
		
			  std::construct_at(reinterpret_cast<T *>(&this->data), data);
          }
        
		template <size_t id>
		AtomData(Types...[id] ::Type *data) : header(id, true) {
			using T = Types...[id] ::Type;
			static_assert(id < sizeof...(Types),
								"Type is not part of Atom, failed to "
								"retrieve TypeId!");
			
			*reinterpret_cast<T**>(&this->data[0]) = data;
		}
		AtomData(header h) : header(h){}
		AtomData(header &&h, size_t length);
	};



	// No arrays of refs
	struct Array {
		template <size_t id> struct CallSizeOf {
            static auto operator()(Atom &elements) -> size_t {
				if(elements.isRef()!=0) return sizeof(void*);
                return Atom::Type<id>::size_of(elements.get_unchecked<id>());}
        };

		const size_t length;
        Atom elements;
		template <size_t id> auto at(size_t idx) -> Atom::Type<id>::Type & {
            using T = Atom::Type<id>::Type;
            assert(!elements.isRef());
			return *(&elements.get_unchecked<id>()+idx);
		}
		template<size_t id>
		auto size_of() -> size_t {
            return sizeof(Array) +
              (length * elements.visitId<CallSizeOf>(elements));
        }
        Array(size_t length, Atom::header h): length(length), elements(h){}
    };
    template <class... Types>
	AtomData<Types...>::AtomData(header &&h, size_t length) : header(ARRAY) {

		auto *a = reinterpret_cast<Array *>(&this->data[0]);
		std::construct_at(a, length, h);
	}

	    struct Cons {
          Atom *car, *cdr;
        };
    struct Table {
        ankerl::unordered_dense::map<Atom, Atom*> val;
    };
    
	using Environment =	ankerl::unordered_dense::map<long long, Atom *>;

	struct Closure {
		Environment env;
		long long expect_args; // negative = rest args
		void *body;
	};
	using Macro = Closure;
    struct Special {
        std::function<bool(Evaluator &, Environment &,
						Atom *, size_t)> func;
		
    };

	
}
