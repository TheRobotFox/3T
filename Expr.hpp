#pragma once

#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <string>

namespace TTT {

	struct SymbolExpr	{
		long		id;
		auto operator==(const SymbolExpr &other) const -> bool = default;
	};
	struct NumberExpr
    {
		double value;
		auto operator==(const NumberExpr& other) const -> bool = default;
	};
	struct IntegerExpr
    {
		int		value;
		auto operator==(const IntegerExpr& other) const -> bool = default;
	};
	struct StringExpr
    {
		std::string		value;
		auto operator==(const StringExpr& other) const -> bool = default;
	};
	struct CharExpr
    {
		char	value;
		auto operator==(const CharExpr& other) const -> bool = default;
	};
    struct InPortExpr
    {
		std::istream value;
		auto operator==(const InPortExpr &other) const -> bool
		{
			return this == &other;
		}

	};
    struct OutPortExpr
	{
		std::ostream value;
		auto operator==(const OutPortExpr &other) const -> bool
		{
			return this == &other;
		}
	};
	struct CallableExpr;

	struct Sexp;
	struct SexpHash {
		auto operator()(const Sexp &s) -> size_t;
	};
	struct SexpEq {
		auto operator()(const Sexp &a,const Sexp &b) -> bool;
	};
	struct HashTableExpr{
		std::unordered_map<Sexp, Sexp, SexpHash, SexpEq> value;
		auto operator==(const HashTableExpr &other) const -> bool;

	};

	struct ConsExpr
	{
		Sexp  *car, *cdr;
		auto operator==(const ConsExpr &other) const -> bool;
			
	};
	struct Null {auto operator==(const Null &_) const -> bool{return true;}};

	using Expr = std::variant<SymbolExpr,
							  NumberExpr,
							  Null,
							  IntegerExpr,
							  StringExpr,
							  CharExpr,
							  CallableExpr,
							  HashTableExpr,
							  ConsExpr>;

	struct Environment {
		using Env = std::map<SymbolExpr, Sexp*>;
		Env env;
		auto lookup(SymbolExpr sym) const -> Sexp*;
		void bind(SymbolExpr sym, Sexp* value){env[sym]=value;}
	};


	struct Macro {
		Sexp *body;
		auto operator==(const Macro &other) const -> bool
		{
			return body == other.body;
		}
		
	};
	
	struct Closure {
		Sexp *body;
		auto operator==(const Closure &other) const -> bool
		{
			return body == other.body;
		}
		// TODO byte code
	};

	struct Interpreter;

	struct Special {
		std::function<bool(Interpreter&, ConsExpr, Environment&, Sexp*)> func;
		auto operator==(const Special &other) const -> bool
		{
			return this == &other;
		}
		
	};
	
	struct CallableExpr {
		Environment env; // TODO vector overlay
		std::vector<SymbolExpr> args;
		std::optional<SymbolExpr> rest;

		std::variant<Closure, Macro, Special> value;
		auto operator==(const CallableExpr &other) const -> bool
		{
			return value == other.value;
		}
	};

	struct Sexp {
		Expr body;
		auto operator==(const Sexp &other) const -> bool;

	};
}

template<>
struct std::hash<TTT::Sexp> {
	auto operator()(const TTT::Sexp &s) -> size_t;
};
template<>
struct std::hash<TTT::HashTableExpr> {
	auto operator()(const TTT::HashTableExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::NumberExpr> {
	auto operator()(const TTT::NumberExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::IntegerExpr> {
	auto operator()(const TTT::IntegerExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::SymbolExpr> {
	auto operator()(const TTT::SymbolExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::ConsExpr> {
	auto operator()(const TTT::ConsExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::StringExpr> {
	auto operator()(const TTT::StringExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::CharExpr> {
	auto operator()(const TTT::CharExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::CallableExpr> {
	auto operator()(const TTT::CallableExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::InPortExpr> {
	auto operator()(const TTT::InPortExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::OutPortExpr> {
	auto operator()(const TTT::OutPortExpr &s) -> size_t;
};
template<>
struct std::hash<TTT::Null> {
	auto operator()(const TTT::Null &s) -> size_t;
};
