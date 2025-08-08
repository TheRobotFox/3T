#include "Special.hpp"
#include "Atom.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <variant>

using namespace TTT;

auto error(Interpreter &interp, Env &env, const std::vector<Atom> &args,
		   Atom *out) -> bool {
	
	const String *msg;
	if ((msg = std::get_if<String>(&args.front())) != nullptr) 
		interp.error = msg->value; 
	else 
		interp.error = "function error expects single String Argument!";
	return false;
}


auto get_car(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out)
	-> bool {
	if (args.size() != 1) {
		interp.error = argument_error(1, args.size());
		return false;
	}
	if (!interp.eval(args[0], env, out))
		return false;
	if (auto *cons = std::get_if<Cons>(out)) {
		*out = *cons->car;
		return true;
	}
	interp.error = std::format("Argument is not a cons Type");
	return false;
}

auto get_cdr(Interpreter &interp, Env &env, const std::vector<Atom>& args, Atom *out)
	-> bool {
	if (args.size() != 1) {
		interp.error = argument_error(1, args.size());
		return false;
	}
	if (!interp.eval(args[0], env, out))
		return false;
	if (auto *cons = std::get_if<Cons>(out)) {
		*out = *cons->cdr;
		return true;
	}
	interp.error = std::format("Argument is not a cons Type");
	return false;
}
auto _if(Interpreter &interp, Env &env, std::vector<Atom> &call, Atom *out)
	-> bool {
	if (call.size() != 4) {
		interp.error = argument_error(3, call.size());
		return false;
	}
	if (!interp.eval(call[1], env, out))
		return false;
	return interp.eval(call[ std::holds_alternative<nil>(*out) ? 3 : 2], env, out);
}


size_t depth = 0;

void reset_reader(HashTable *readtable, Env &env) {
	depth = 0;
	if (env.contains(InternalSymbols::backup)) {
		readtable->value[Char{')'}] = env[InternalSymbols::backup];
		env.erase(InternalSymbols::backup);
	}
}


auto read_delimeter(Interpreter &interp, Env &env, const std::vector<Atom> &args,
					Atom *out) -> bool {

	HashTable *readtable;
	if ((readtable = std::get_if<HashTable>(interp.mod.readtable)) == nullptr) {
		interp.error = "Expected *readtable* to be an Table";
		depth = 0;
		return false;
	}

	// Parse Arguments
	Char closing;
	if (const auto *c = std::get_if<Char>(&args.at(1))) {
		closing = *c;
	} else {
		interp.error = "Expected Second Argument to be Char!";
		return false;
	}
		
	// Backup and set Readtable
	if (readtable->value.contains(closing))
		env[InternalSymbols::backup] = readtable->value[closing];
	readtable->value[closing] = env[InternalSymbols::eof_func];


	depth++;
	
	// Read Cons-Cells

	*out = nil{};
	Atom *current = out;

	while(true) {
		Atom *car = interp.mod.memory.alloc();
		if (!interp.eval(Call{.head = interp.mod.f_read, .args = {}},
						  interp.mod.global, car)){
			depth = 0;
			reset_reader(readtable, env);
			return false;
		}
		if (*car == Atom{Symbol{InternalSymbols::eof}})
			break;
			
		Atom *next = interp.mod.memory.alloc();

		*current = Cons{.car=car, .cdr=next};
		current = next;
	}
	
	depth--;
	if(depth == 0) reset_reader(readtable, env);
	return true;
}

auto read_whitespace(Interpreter &interp, Env &env,
					 const std::vector<Atom> &args, Atom *out) -> bool {
	InPort *inp;
	if ((inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
		interp.error = "in-port is not an InPort!";
		return false;
	}

	char c;
	while(true) {
		c = inp->value->peek();
		if (c != ' ' && c != '\t' && c != '\n')
			break;
		inp->value->get();
	}
	return interp.eval(Call{.head = interp.mod.f_read, .args = {}},
				  interp.mod.global, out);
}

auto read_comment(Interpreter &interp, Env &env, const std::vector<Atom> &args,
				  Atom *out) -> bool {
	InPort *inp;
	if ((inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
		interp.error = "in-port is not an InPort!";
		return false;
	}

	while (inp->value->get() != '\n')
		;
	
	return interp.eval(Call{.head = interp.mod.f_read, .args = {}},
				  interp.mod.global, out);	
}

auto read_string(Interpreter &interp, Env &env,
				 const std::vector<Atom> &args, Atom *out) -> bool {
	InPort *inp;
	if ((inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
		interp.error = "in-port is not an InPort!";
		return false;
	}
	char c;
	std::string res;
	while (true) {
		c = inp->value->get();
		if (c == '\\')
			c = inp->value->get();
		else if (c == '\"')
			break;
		res += c;
	}
	*out = String{std::move(res)};
	return true;
}

auto read_char(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool {
	InPort *inp;
	if ((inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
		interp.error = "in-port is not an InPort!";
		return false;
	}	
	char c = inp->value->get();
	*out = Char{.value = c};
	return true;
}

auto read(Interpreter &interp, Env &env, const std::vector<Atom> &args,
		  Atom *out) -> bool {
	auto &readTable = interp.mod.readTable;
	InPort *inp;
	if (inp->value->eof()) {
		if (depth != 0) {
			interp.error = "Unbalanced_parents";
			depth = 0; // FIXME make depth and backup local environemnt to support parens and brackets simultan;
			return false;
		}
		*out = nil{};
		return true;
	}
	char c = inp->value->get();
	// Handle Read Table 
	if (readTable.contains(Char{c}))
		return interp.eval(*readTable[Char{c}], interp.mod.global,
					  out);

	// Handle Numbers
	if (isdigit(c) != 0) {
		bool real = false;
		std::string num {c};
		while (true) {
			c = inp->value->peek();
			if ((isdigit(c) != 0) || (c == '.' && !real)) 
				num += inp->value->get();
			else 
				break;
		}
		if (real)
			*out = Real{std::strtod(num.c_str(), NULL)};
		else
			*out = Integer{std::strtoll(num.c_str(), NULL, 10)};
		return true;
	}

	// Interpret Symbol otherwiese
	std::string symbol{c};

	while (true) {
		char c = inp->value->peek();
		if (readTable.contains(Char{c}) || inp->value->eof())
			break;

		symbol += inp->value->get();
	}
	*out = String{symbol};
	return true;
}
