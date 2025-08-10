#pragma once
#include <cstddef>
#include <fstream>
#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <variant>
#include <concepts>

namespace TTT {
	using SymbolId = long;
	class Memory;
	struct Interpreter;


	// Literals

	template <class T> struct LitImpl;

	using	String	= LitImpl<std::string>;
	using	Char	= LitImpl<char>;
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
	
	struct nil;
	struct t;
	struct InPort;
	struct OutPort;
	struct Symbol;

	

	using Atom =
		std::variant<t, nil, String, Char, Integer, Real, Symbol, Quoted, Cons, HashTable,
					 Closure, Macro, Special, Call, InPort, OutPort>;


	using Env = std::unordered_map<SymbolId, Atom *>;
	struct Cons_iterator;

} // namespace TTT
using namespace TTT;


struct Printer;
std::ostream &operator<<(std::ostream &os, Atom const &m);




/****************************************************
 *					Literals						*
 ****************************************************/

template <class T> struct TTT::LitImpl {
	T value;
	auto operator==(const LitImpl<T> &other) const -> bool {return value == other.value;}
};
struct TTT::t {
	auto operator==(const t &_) const -> bool { return true; }
};
struct TTT::nil {
	auto operator==(const nil&_) const -> bool {return true;}
};

template <class T> struct std::hash <LitImpl<T>> {
	auto operator()(const LitImpl<T> &l) -> size_t {
		return std::hash<T>{}(l.value);
	}
};
template<> struct std::hash<t> {
	auto operator()(const t &_)		-> size_t {return-1;};
};
template<> struct std::hash<nil> {
	auto operator()(const nil &_)	-> size_t {return 0;};
};





/****************************************************
 *					Miscellaneously					*
 ****************************************************/

struct TTT::Symbol {
	SymbolId id;
	auto operator==(const Symbol &other) const -> bool = default;
};

struct TTT::InPort {
	std::istream *value;
	auto operator==(const InPort &other) const -> bool {
		return value == other.value;
	}
	~InPort(){delete value;}
};
struct TTT::OutPort {
	std::ostream *value;
	auto operator==(const OutPort &other) const -> bool{
		return value == other.value;
	}
	~OutPort(){delete value;}
};

template<> struct std::hash<Symbol> {
	auto operator()(const Symbol &c)	-> size_t;	
};
template<> struct std::hash<InPort> {
	auto operator()(const InPort &c)	-> size_t;	
};
template<> struct std::hash<OutPort> {
	auto operator()(const OutPort &c) -> size_t;
};





/****************************************************
 *					Procedures						*
 ****************************************************/

struct Callable {
	auto operator==(const Callable &other) const -> bool {
		return this==&other;
	}
};

struct TTT::Closure {
	Atom *body; /* byte code */
	Env env;
	std::vector<SymbolId> args;
	SymbolId rest;
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
	std::function<bool(Interpreter &, Env &, const std::vector<Atom> &, Atom *)> func;
	Env env;
	
	auto operator==(const Special &other) const -> bool {
		return this == &other;
	}
};

struct TTT::Call {
	Atom *head;
	std::vector<Atom> args;
	auto operator==(const Call &other) const -> bool {
		return this == &other;
	}
};


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
 *					Containers						*
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
	auto operator==(const Cons &other) const -> bool;
};

// struct TTT::Cons_iterator {
//	Atom *current;
//	auto next() -> Atom * {
//		if (auto *cons = std::get_if<Cons>(current)) {
//			current = cons->cdr;
//			return cons->car;
//		}
//		return nullptr;
//	}
//	auto count() -> size_t {
//		size_t length = 0;
//		while (next() != nullptr)
//			length++;
//		return length;
//	}
// };

struct TTT::HashTable {
	HashTable(std::unordered_map<Atom, Atom *> &&value)
	: value(new std::unordered_map(value)), own(true) {}
	HashTable(std::unordered_map<Atom, Atom *> &value) : value(&value), own(false){}
	HashTable(HashTable &&other) : value(other.value), own(other.own) {}
	HashTable(const HashTable &other) : own(other.own) {
		if (own)
			value = new std::unordered_map(*other.value);
		else
			value = other.value;
	}
	auto operator=(const HashTable &other) noexcept -> HashTable & {
		if (own)
			value = new std::unordered_map(*other.value);
		else
			value = other.value;
		
		return *this;
	}
	auto operator=(HashTable &&other) noexcept -> HashTable & {
		own = other.own;
		value = other.value;
		
		return *this;
	}
	std::unordered_map<Atom, Atom *> *value;
	bool own;
	void mark_children(Memory &mem) const;
	auto operator==(const HashTable &other) const -> bool;
	~HashTable() {
		if (own)
			delete value;
	}

};
