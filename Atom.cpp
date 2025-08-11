#include "Interpreter.hpp"
#include "Printer.hpp"
#include <cstddef>
#include <format>
#include <variant>
#include <vector>

using namespace TTT;



std::ostream &operator<<(std::ostream &os, Atom const &m) {
	m.visit(Printer{os});
	return os;
}


template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}



/****************************************************
 *					Literals						*
 ****************************************************/



/****************************************************
 *					Miscellaneously					*
 ****************************************************/


auto std::hash<Symbol>::operator()(const Symbol &c)		-> size_t {
	return std::hash<SymbolId>{}(c.id);
}
auto std::hash<Quoted>::operator()(const Quoted &c) -> size_t {
	size_t res = std::hash<SymbolId>{}(c.sym);
	hash_combine(res, c.depth);
	return res;
}
auto std::hash<InPort>::operator()(const InPort &c)		-> size_t {
	return std::hash<size_t>{}((size_t)&c);
}
auto std::hash<OutPort>::operator()(const OutPort &c)	-> size_t {
	return std::hash<size_t>{}((size_t)&c);
}




/****************************************************
 *					Procedures                      *
 ****************************************************/

// TODO better hashing functions

auto std::hash<Closure>::operator()(const Closure &c)	-> size_t {
	return std::hash<size_t>{}((size_t)c.body);
		}
auto std::hash<Macro>::operator()(const Macro &c)		-> size_t {
	return std::hash<size_t>{}((size_t)c.body);
}
auto std::hash<Special>::operator()(const Special &c)	-> size_t {
	return std::hash<size_t>{}((size_t)&c);
}
auto std::hash<Call>::operator()(const Call &c)			-> size_t {
	return std::hash<size_t>{}((size_t)&c);
}




/****************************************************
 * 					Containers						*
 ****************************************************/

auto std::hash<Cons>::operator()(const Cons &c) -> size_t {
	size_t res = std::hash<Atom>{}(*c.car);
	hash_combine(res, *c.cdr);
	return res;
}
auto std::hash<HashTable>::operator()(const HashTable &ht) -> size_t {
	size_t res = 0;
	for (const auto &[k, v] : *ht.value) {
		hash_combine(res, k);		
		hash_combine(res, v);
	}
	return res;
}

void Cons::mark_children(Memory &memory) const {
	memory.mark(car);
	memory.mark(cdr);
}
auto Cons::operator==(const Cons &other) const -> bool {
	return this == &other || (*car == *other.car && *cdr == *cdr);
}


void HashTable::mark_children(Memory &memory) const {
	for(const auto &[k, v] : *value) memory.mark(v);
}

auto HashTable::operator==(const HashTable &other) const -> bool {
	return *value == *other.value;
}

