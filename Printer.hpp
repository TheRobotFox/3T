#pragma once
#include "Atom.hpp"
#include "Module.hpp"


namespace TTT {
	std::ostream &operator<<(std::ostream &os, Atom const &m);

	struct PrintType {
		constexpr auto operator()(const OutPort		&_) const -> std::string{return "OutPort"		;}
		constexpr auto operator()(const InPort		&_) const -> std::string{return "InPort"		;}
		constexpr auto operator()(const Call		&_) const -> std::string{return "Call"			;}
	    constexpr auto operator()(const Special		&_) const -> std::string{return "Special"		;}
	    constexpr auto operator()(const Macro		&_) const -> std::string{return "Macro"		;}
	    constexpr auto operator()(const Closure		&_) const -> std::string{return "Closure"		;}
	    constexpr auto operator()(const HashTable	&_) const -> std::string{return "HashTable"	;}
	    constexpr auto operator()(const Cons		&_) const -> std::string{return "Cons"			;}
	    constexpr auto operator()(const Quoted		&_) const -> std::string{return "Quoted"		;}
	    constexpr auto operator()(const Symbol		&_) const -> std::string{return "Symbol"		;}
	    constexpr auto operator()(const Real		&_) const -> std::string{return "Real"			;}
	    constexpr auto operator()(const Integer		&_) const -> std::string{return "Integer"		;}
	    constexpr auto operator()(const Char		&_) const -> std::string{return "Char"			;}
	    constexpr auto operator()(const String		&_) const -> std::string{return "String"		;}
	    constexpr auto operator()(const t			&_) const -> std::string{return "t"			;}
	    constexpr auto operator()(const nil			&_) const -> std::string{return "nil"			;}
	};

	struct Printer {
		std::ostream &out;
		Module *mod {};
		template <class T> void operator()(const LitImpl<T> &l) {
			out << l.value;
		}
		template <class T> void operator()(const T &a) {
			out << "[" << PrintType{}(a) << "]";
		}
		void operator()(const Cons &c) {
			out << '(';
			c.car->visit(*this);
			out << ' ';
			c.cdr->visit(*this);
			out << ')';
		}
		void operator()(const Call &c) {
			out << '(';
			c.head->visit(*this);
			out << "<--";
			for (const Atom &a : c.args){
				out << ' ';
				a.visit(*this);
			}
			out << ')';

		}
		void operator()(const nil &c) {
			out << "nil";
		}
		void operator()(const t &c) {
			out << "t";
		}
		void operator()(const Symbol &c) {
			if (mod != nullptr) 
				out << mod->get_symbol_name(c.id);
			else
				out << "[Unknown Symbol " << c.id << "]";
		}
		void operator()(const Quoted &c) {
			for (unsigned int i = 0; i <= c.depth; i++)
				out << "'";
			this->operator()(Symbol{c.sym});
		}
	};
}
