#include "Expr.hpp"
#include <cstddef>
#include <functional>
#include <variant>

using namespace TTT;

auto ConsExpr::operator==(const ConsExpr &other) const -> bool
{
	return (car == other.car || *car == *other.car)
	  && (cdr == other.cdr || *cdr == *other.cdr);
}
auto Sexp::operator==(const Sexp &other) const -> bool
{
	return this == &other || body == other.body;
}

auto HashTableExpr::operator==(const HashTableExpr &other) const -> bool
{
	return value == other.value;
}	 

auto SexpHash::operator()(const Sexp &s) -> size_t {
  return std::hash<Sexp>{}(s);
}

auto std::hash<Sexp>::operator()(const Sexp &s) -> size_t
{
	return std::hash<Expr>{}(s.body);
}	 

auto std::hash<NumberExpr>::operator()(const NumberExpr &s) -> size_t
{
	return std::hash<decltype(s.value)>{}(s.value);
}
auto std::hash<HashTableExpr>::operator()(const HashTableExpr &s) -> size_t
{
	size_t res = 0;
	for(const auto &[k, v] : s.value){
		hash_combine(res, k);
		hash_combine(res, v);
	}
	return res;
}

auto std::hash<IntegerExpr>::operator()(const IntegerExpr &s) -> size_t
{
	return std::hash<decltype(s.value)>{}(s.value);
}

auto std::hash<SymbolExpr>::operator()(const SymbolExpr &s) -> size_t
{
	return std::hash<decltype(s.id)>{}(s.id);	
}

auto std::hash<Quoted>::operator()(const Quoted &s) -> size_t
{
	return std::hash<decltype(s.sym.id)>{}(~(s.sym.id));
}

auto std::hash<ConsExpr>::operator()(const ConsExpr &s) -> size_t
{
	size_t res = std::hash<Sexp>{}(*s.car);
	hash_combine(res, std::hash<Sexp>{}(*s.cdr));
	return res;
}

auto std::hash<StringExpr>::operator()(const StringExpr &s) -> size_t
{
	return std::hash<decltype(s.value)>{}(s.value);	
}

auto std::hash<CharExpr>::operator()(const CharExpr &s) -> size_t
{
	return std::hash<decltype(s.value)>{}(s.value);	
}

auto std::hash<CallableExpr>::operator()(const CallableExpr &s) -> size_t
{
	return std::hash<size_t>{}((size_t)&s); // FIXME better hash?
}

auto std::hash<InPortExpr>::operator()(const InPortExpr &s) -> size_t
{
	return std::hash<size_t>{}((size_t)&s);
}

auto std::hash<OutPortExpr>::operator()(const OutPortExpr &s) -> size_t
{
	return std::hash<size_t>{}((size_t)&s);	
}

auto std::hash<Null>::operator()(const Null &_) -> size_t
{
	return 0;
}

auto SexpEq::operator()(const Sexp &a,const Sexp &b) -> bool
{
	return a==b;
}


auto Environment::lookup(SymbolExpr sym) const -> Sexp*
{
	if (auto v = local.find(sym); v!=local.end())
		return v->second;
	if (auto v = global.find(sym); v!=global.end())
		return v->second;
	return nullptr;
}

