#include "Atom.hpp"
#include <ankerl/unordered_dense.h>
#include <cstddef>
#include <format>
#include <variant>
#include <vector>

using namespace TTT;



/****************************************************
 *					Literals						*
 ****************************************************/



auto std::hash<Symbol>::operator()(const Symbol &c)	-> size_t {
	return ankerl::unordered_dense::detail::wyhash::hash(c.id);
}
auto std::hash<Quoted>::operator()(const Quoted &c) -> size_t {
	size_t res = ankerl::unordered_dense::detail::wyhash::hash(c.sym);
	return ankerl::unordered_dense::detail::wyhash::mix(res, c.depth);
}



/****************************************************
 *					Miscellaneously					*
 ****************************************************/

auto Forward::operator==(const Forward &other) const -> bool {
	return *reference==*other.reference;
}
auto std::hash<Forward>::operator()(const Forward &fw) -> size_t {
	return std::hash<Atom>{}(*fw.reference);
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




/****************************************************
 * 					Containers						*
 ****************************************************/

auto std::hash<Cons>::operator()(const Cons &c) -> size_t {
	return ankerl::unordered_dense::detail::wyhash::mix(std::hash<Atom>{}(*c.car),
                                                   std::hash<Atom>{}(*c.cdr));
}
auto std::hash<HashTable>::operator()(const HashTable &ht) -> size_t {
	size_t res = 0;
	for (const auto &[k, v] : ht.value) {
		res = ankerl::unordered_dense::detail::wyhash::mix(res, std::hash<Atom>{}(k));
		res = ankerl::unordered_dense::detail::wyhash::mix(res, std::hash<Atom>{}(*v));
	}
	return res;
}

// void Cons::mark_children(Memory &memory) const {
// 	memory.mark(car);
// 	memory.mark(cdr);
// }
auto Cons::operator==(const Cons &other) const -> bool {
	return this == &other || (*car == *other.car && *cdr == *cdr);
}


// void HashTable::mark_children(Memory &memory) const {
// 	for(const auto &[k, v] : value) memory.mark(v);
// }

auto HashTable::operator==(const HashTable &other) const -> bool {
	return value == other.value;
}

template<class T> struct _AtomHasher_helper{};

struct AtomHasher {
	template <class T> constexpr auto operator()(const T &v) -> size_t {
		_AtomHasher_helper<typename std::hash<T>::is_avalanching> _;
		return std::hash<T>{}(v);
	}
};

auto std::hash<Atom>::operator()(const Atom &at) -> size_t {
	return ankerl::unordered_dense::detail::wyhash::mix(
												   ankerl::unordered_dense::detail::wyhash::hash(at.index()),
												   at.visit(AtomHasher{}));
}
