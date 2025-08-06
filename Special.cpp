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
	if (call.size() != 1) {
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



auto read_delimeter(Interpreter &interp, Env &env, const std::vector<Atom> &call,
					Atom *out) -> bool {
	// get closing delimeter from env

	char closing;
	if (auto *c = std::get_if<Char>(env[interp.mod.intern("closing")])) {
		closing = c->value;
	} else {
		interp.error = "expected 'closing to be Char!"; // TODO better Error
		return false;
	}
	static long s_backup = interp.mod.intern("backup");
	static long s_eof	 = interp.mod.intern("eof-func");
	static size_t depth = 0;

	HashTable *readtable;

	if ((readtable = std::get_if<HashTable>(interp.mod.readtable)) == nullptr) {
		interp.error = "Expected *readtable* to be an Table";
		return false;
	}
	depth++;
		
	if (readtable->value.contains(Char{closing}))
		env[s_backup] = readtable->value[Char{closing}];

	readtable->value[Char{closing}] = env[s_eof];

	*out = nil{};
	Atom *current = out;

	while(true) {
		Atom *car = interp.mod.memory.alloc();
		if (!interp.eval(Call{.head = interp.mod.read, .args = {}},
						  interp.mod.global, car))
			goto cleanup;
		if (*car == Atom{Symbol{interp.mod.eof}})
			break;
			
		Atom *next = interp.mod.memory.alloc();

		*current = Cons{.car=car, .cdr=next};
		current = next;
	}
	depth--;
	if(depth == 0) ; // TODO restore readtable
	return true;
	
 cleanup:
	depth = 0;
	//TODO restore readTable
	return false;
}

// TODO: translate read hardcoded into readtable
auto read_whitespace(Interpreter &interp, Env &env,
					 const std::vector<Atom> &call, Atom *out) -> bool {
	if(auto *inp = std::get_if<InPort>(args))
}



	auto read(Interpreter &interp, Env &env, const std::vector<Atom> &call,
			  Atom *out) -> bool {
		HashTable *readtable;
		InPort *inp;
		if ((readtable = std::get_if<HashTable>(interp.mod.readtable)) ==
			nullptr ||
			(inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
			interp.error = "Expected *readtable* to be an Table and *in-port* to be an InPort";
			return false;
		}
		// skip whitespace
		char c;
		while (true) {
			c = inp->value->get();
			if ((c == ' ' || c == '\t' || c == '\n'))
				continue;
			if (c != ';') break;
			while((c = inp->value->get()) != '\n');
		}


		// Handle Read Table 
		if (readtable->value.contains(Char{c}))
			return interp.eval(*readtable->value[Char{c}], interp.mod.global,
						  out);

		// Handle Numbers
		if (isdigit(c) != 0) {
			bool real = false;
			std::string num = {c};
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

		// Handle String
		if (c == '\"') {
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

		// Handle Char
		if (c == '?') {
			c = inp->value->get();
			*out = Char{.value = c};
			return true;
		}


		// Interpret Symbol otherwiese
	
	
		return true;
	}
