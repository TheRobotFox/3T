
#include <cstddef>
#include <functional>
#include <istream>
#include <unordered_map>
#include <variant>
#include <concepts>

namespace TTT {
	using SymbolId = long;
	struct Memory;
	struct Interpreter;


	// Literals

	template <class T> struct LitImpl;

	using	String	= LitImpl<std::string>;
	using	Integer = LitImpl<long long>;
	using	Real	= LitImpl<double>;
	using	Quoted	= LitImpl<SymbolId>;


	// Containers
	
	struct Cons;
	struct HashTable;


	// Procedures
	struct Closure;
	struct Macro;
	struct Special;
	struct Call;

	
	// Miscellaneously
	
	struct nil {};
	struct t   {};
	struct InPort;
	struct OutPort;
	struct Symbol;



	using Atom =
		std::variant<t, nil, String, Integer, Real, Symbol, Quoted, Cons, HashTable,
					 Closure, Macro, Special, Call, InPort, OutPort>;


	struct Environment {

		std::string name;
		std::unordered_map<SymbolId, Atom *> bindings;
		Environment *parent;
	};


} // namespace TTT
using namespace TTT;



/****************************************************
 *					Literals						*
 ****************************************************/

template <class T> struct TTT::LitImpl {
	T value;
};

template <class T> struct std::hash <LitImpl<T>> {
	auto operator()(const LitImpl<T> &l) -> size_t {
		return std::hash<T>{}(l.value);
	}
};





/****************************************************
 *					Miscellaneously					*
 ****************************************************/

struct TTT::Symbol {
	SymbolId id;
	auto eval(Interpreter &interp, Env &env) const -> Atom*;		
};

struct TTT::InPort  {std::istream value;};
struct TTT::OutPort {std::ostream value;};

template<> struct std::hash<Symbol> {
  auto operator()(const Symbol &c)		-> size_t;	
};
template<> struct std::hash<InPort> {
  auto operator()(const InPort &c)		-> size_t;	
};
template<> struct std::hash<OutPort> {
  auto operator()(const OutPort &c)		-> size_t;
};
template<> struct std::hash<t> {
	auto operator()(const t &_)	-> size_t {return-1;};
};
template<> struct std::hash<nil> {
	auto operator()(const nil &_)	-> size_t {return 0;};
};





/****************************************************
 *					Procedures						*
 ****************************************************/

struct Callable {
	Environment env;
	std::vector<SymbolId> args;
	SymbolId rest;
};

struct TTT::Closure : Callable {Atom *body; /* byte code */	};
struct TTT::Macro   : Callable {Atom *body;					};
struct TTT::Special : Callable {							};

struct TTT::Call {Atom *head; Atom *args;};


template<> struct std::hash<Closure> {
  auto operator()(const Closure &c) -> size_t;	
};
template<> struct std::hash<Macro> {
  auto operator()(const Macro &c)	-> size_t;	
};
template<> struct std::hash<Special> {
  auto operator()(const Special &c) -> size_t;	
};
template<> struct std::hash<Call> {
  auto operator()(const Call &c)	-> size_t;
};





/****************************************************
 * 					Containers						*
 ****************************************************/

template <> struct std::hash<Cons> {
  auto operator()(const Cons &c) -> size_t;
};

template <> struct std::hash<HashTable> {
  auto operator()(const HashTable &ht) -> size_t;
};

struct TTT::Cons {
	Atom *car, *cdr;
	void mark_children(Memory &mem) const;
};

struct TTT::HashTable {
	std::unordered_map<Atom, Atom *> value;
	void mark_children(Memory &mem) const;
};
