#pragma once
#include <ankerl/unordered_dense.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <variant>

namespace TTT {
	using SymbolId = long;
	class GC;
	struct Interpreter;


	// Literals
	template <class T> struct LitImpl;

	using	Char	= LitImpl<char>;
	using	Integer = LitImpl<long long>;
	using	Real = LitImpl<double>;
	using	String	= LitImpl<std::string>;

	struct Symbol;
	struct Quoted;

	struct t;
	struct nil;


	// Containers
    template <class Fn> struct CallChildren;

	struct Cons;
	struct HashTable;


	// Procedures
	struct Closure;
	struct Macro;
	struct Special;

	
	// Miscellaneously
	struct Forward;

	using Atom = std::variant<nil, t, String, Char, Integer, Real, Symbol, Quoted,
							  Cons, HashTable, Closure, Macro, Special, Forward>; 

	

	using Env = std::unordered_map<SymbolId, Atom *>;
	struct Cons_iterator;

} // namespace TTT
using namespace TTT;

template <> struct std::hash<Atom> {
	using is_avalanching = void;	
	auto operator()(const Atom &at) -> size_t;
};




/****************************************************
 * 						Literals					*
 ****************************************************/


template <class T> struct TTT::LitImpl {
	T value;
	auto operator==(const LitImpl<T> &other) const -> bool {return value == other.value;}
};

struct TTT::Symbol {
	SymbolId id;
	auto operator==(const Symbol &other) const -> bool = default;
};
 
struct TTT::Quoted {
	SymbolId sym;
	unsigned int depth;
	auto operator==(const Quoted &other) const -> bool = default;
};

template <> struct std::hash<Symbol> {
	using is_avalanching = void;
	auto operator()(const Symbol &c)	-> size_t;	
};
template <> struct std::hash<Quoted> {
	using is_avalanching = void;
	auto operator()(const Quoted &c)	-> size_t;
};

struct TTT::t {
	auto operator==(const t &_) const -> bool { return true; }
};
struct TTT::nil {
	auto operator==(const nil&_) const -> bool {return true;}
};


template <class T> struct std::hash<LitImpl<T>> {
	using is_avalanching = void;
	auto operator()(const LitImpl<T> &l) -> size_t {
		return std::hash<T>{}(l.value);
	}
};
template <> struct std::hash<t> {
	using is_avalanching = void;
	auto operator()(const t &_)		-> size_t {return 0x589965cc75374cc3;};
};
template <> struct std::hash<nil> {
	using is_avalanching = void;
	auto operator()(const nil &_)	-> size_t {return 0x8ebc6af09c88c6e3;};
};





/****************************************************
 *					Miscellaneously					*
 ****************************************************/


struct TTT::Forward {
	Atom *reference;
	auto operator==(const Forward &other) const -> bool;
};

template <> struct std::hash<Forward> {
	using is_avalanching = void;
	auto operator()(const Forward &fw) -> size_t;
};




/****************************************************
 *					Procedures						*
 ****************************************************/


struct TTT::Closure {
	Atom *body; /* byte code */
	Env env;
	std::vector<SymbolId> args;
	SymbolId rest{0};
	auto operator==(const Closure &other) const -> bool {return body == other.body;}
};

struct TTT::Macro {
	Atom *body;
	Env env;
	std::vector<SymbolId> args;
	SymbolId rest;
	auto operator==(const Macro	  &other) const -> bool  {return body == other.body;}
};

struct TTT::Special {
	std::function<bool(Interpreter &, Env &, Env &, const std::vector<Atom> &, Atom *)> func;
	Env env;
	
	auto operator==(const Special &other) const -> bool {
		return this == &other;
	}
};

template <> struct std::hash<Closure> {
	using is_avalanching = void;
	auto operator()(const Closure &c) -> size_t;	
};
template <> struct std::hash<Macro> {
	using is_avalanching = void;
	auto operator()(const Macro &c)	-> size_t;
};
template <> struct std::hash<Special> {
	using is_avalanching = void;
	auto operator()(const Special &c) -> size_t;	
};





/****************************************************
 *					Containers						*
 ****************************************************/

template <> struct std::hash<Cons> {
	using is_avalanching = void;
	auto operator()(const Cons &c) -> size_t;
};

template <> struct std::hash<HashTable> {
	using is_avalanching = void;
	auto operator()(const HashTable &ht) -> size_t;
};

struct TTT::Cons {
	Atom *car, *cdr;
	auto operator==(const Cons &other) const -> bool;
};

struct TTT::HashTable {
	ankerl::unordered_dense::map<Atom, Atom *> value;
	auto operator==(const HashTable &other) const -> bool;
};
template <class Fn> struct TTT::CallChildren {
	Fn op;
	template <class T> void operator()(const T &_) const {}
    
	void operator()(HashTable &ht) const {
		for (auto &[k, v] : ht.value) {
			k.visit(*this);
			op(v);
		}
	}
	void operator()(Cons &c) const {
		op(c.car);
		op(c.cdr);
	}

    CallChildren(const Fn &fn) : op(fn){}
};
