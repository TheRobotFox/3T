#pragma once
#include "Atom.hpp"
#include "Module.hpp"


std::ostream &operator<<(std::ostream &os, Atom const &m);

struct Printer {
	std::ostream &out;
	Module *mod {};
	template <class T> void operator()(const LitImpl<T> &l) {
		out << l.value;
	}
	template <class T> void operator()(const T &a) {
		out << "["<< typeid(T).name() << "]";
	}
	void operator()(const Cons &c) {
		out << '(';
		c.car->visit(*this);
		out << ' ';
		c.cdr->visit(*this);
		out << ')';
	}
	void operator()(const Call &c) {
		c.head->visit(*this);
		out << "<--";
		for (const Atom &a : c.args){
			out << ' ';
			a.visit(*this);
		}
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
		for (unsigned int i = 0; i < c.depth; i++)
			out << "'";
		this->operator()(Symbol{c.sym});
	}
};
