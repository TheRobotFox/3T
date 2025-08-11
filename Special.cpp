#include "Special.hpp"
#include "Atom.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <variant>


namespace TTT {

	auto argument_error(int expected, int got) -> std::string {
		return std::format("Expected {} Arguments got {}", expected, got);
	}
	auto error(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool {

		const String *msg;
		if (!env.empty() && ((msg = std::get_if<String>(env.begin()->second)) != nullptr))
			interp.error = msg->value;
		else if ((msg = std::get_if<String>(&args.front())) != nullptr) 
			interp.error = msg->value; 
		else 
			interp.error = "function error expects single String Argument!";
		return false;
	}

	auto get_car(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args, Atom *out)
	-> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}
		if (!interp.eval(args[0], called_from, out))
			return false;
		if (auto *cons = std::get_if<Cons>(out)) {
			*out = *cons->car;
			return true;
		}
		interp.error = std::format("Argument is not a cons Type");
		return false;
	}

	auto get_cdr(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom>& args, Atom *out)
	-> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}
		if (!interp.eval(args[0], called_from, out))
			return false;
		if (auto *cons = std::get_if<Cons>(out)) {
			*out = *cons->cdr;
			return true;
		}
		interp.error = std::format("Argument is not a cons Type");
		return false;
	}
	
	auto _if(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args, Atom *out)
	-> bool {
		if (args.size() != 3) {
			interp.error = argument_error(3, args.size());
			return false;
		}
		if (!interp.eval(args[0], called_from, out))
			return false;
		return interp.eval(args[ std::holds_alternative<nil>(*out) ? 2 : 1], called_from, out);
	}


	size_t depth = 0;

	// Args [closing_char, unbalanced_delimeter_error_function]
	// Env [eof_returning_function]
	auto read_delimeter(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
						Atom *out) -> bool {

		if (args.size() != 2) {
			interp.error = argument_error(2, args.size());
			return false;
		}
		// Parse Arguments
		Char closing;
		if (const auto *c = std::get_if<Char>(&args.at(0))) {
			closing = *c;
		} else {
			interp.error = "Expected Second Argument to be Char!";
			return false;
		}

		
		if (depth == 0){
			env[InternalSymbols::reader_backup_fun] = interp.mod.readTable[Char{closing}];
			interp.mod.readTable[Char{closing}] =
		      env[InternalSymbols::reader_eof_func];
		}
		
		depth++;
		bool ret = false;
	
		// Read Cons-Cells
		*out = nil{};
		Atom *current = out;

		while(true) {
			Atom *car = interp.mod.memory.alloc();
			if (!interp.eval(Call{.head = interp.mod.f_read, .args = {}},
							  interp.mod.global, car)){
				depth = 0;
				interp.error += "\nRead Error!";
				goto end;
			}
			if (*car == Atom{Quoted{InternalSymbols::eof}})
				break;
			
			Atom *next = interp.mod.memory.alloc();

			*current = Cons{.car=car, .cdr=next};
			current = next;
		}

		depth--;
		ret = true;
		
		if (depth == 0) {
		end:
			interp.mod.readTable[Char{closing}] = env[InternalSymbols::reader_backup_fun];
		}
		return ret;
	}

	auto read_whitespace(Interpreter &interp, Env &called_from, Env &env,
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

	auto read_comment(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
					  Atom *out) -> bool {
		InPort *inp;
		if ((inp = std::get_if<InPort>(interp.mod.in_port)) == nullptr) {
			interp.error = "in-port is not an InPort!";
			return false;
		}
		while (inp->value->get() != '\n' && !inp->value->eof())
			;
		return interp.eval(Call{.head = interp.mod.f_read, .args = {}},
					  interp.mod.global, out);	
	}

	auto read_string(Interpreter &interp, Env &called_from, Env &env,
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

	auto read_char(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
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

	auto read(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool {
		auto &readTable = interp.mod.readTable;
		InPort *inp;
		if ((inp = std::get_if<InPort>(interp.mod.in_port)) ==
			nullptr) {
			interp.error = "Stdin Port is not a InPort!";
			return false;
		}
		if (inp->value->eof()) {
			if (depth != 0) {
				interp.error = "Missing closing Parenth";
				depth = 0; // FIXME make depth and backup local environemnt to support parens and brackets simultan;
				return false;
			}
			*out = nil{};
			return true;
		}
		char c = inp->value->get();
		// Handle Read Table 
		if (readTable.contains(Char{c}))
			return interp.eval(Call{.head = readTable[Char{c}], .args = {}}, interp.mod.global,
						  out);

		// Handle Numbers
		if (isdigit(c) != 0) {
			bool real = false;
			std::string num {c};
			while (true) {
				c = inp->value->peek();
				if ((isdigit(c) != 0) || (c == '.' && !real)) {
					num += inp->value->get();
					if(c=='.') real = true;
				} else 
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
		*out = Symbol{interp.mod.intern(symbol)};
		return true;
	}

	struct Quoting {
		Interpreter &interp;
		const Atom &self;
		Atom *out;
		Env &env;
		bool evaled = false;

		template <class T> auto operator()(const T &_) const -> bool {
			if (!evaled){
				if (!interp.eval(self, env, out))
					return false;
				
				return out->visit(Quoting{.interp = interp, .self = *out, .out = out, .env = env, .evaled = true});
			}
			*out = self;
			return true;
		}
		auto operator()(const Symbol &sym) const -> bool {
			*out = Quoted{.sym = sym.id, .depth = 0};
			return true;			
		}
		auto operator()(const Quoted &q) const -> bool {
			*out = Quoted{.sym = q.sym, .depth = q.depth + 1};
			return true;			
		}
	};

	auto quote(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}

		args[0].visit(Quoting{interp, args[0], out, called_from, false});
		return true;
	}

	auto list(Interpreter &interp, Env &called_from, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool {
		Atom *current = out;
		*current = nil{};
		for (const Atom &a : args) {
			Atom *car = interp.mod.memory.alloc();
			if (!interp.eval(a, called_from, car))
				return false;
			Atom *next = interp.mod.memory.alloc();
			*current   = Cons{car, next};
			current	   = next;
		}
		return true;
	}

	struct Eval {
		Interpreter &interp;
		const Atom &self;
		Atom *out;

		auto operator()(const Quoted &q) const -> bool {
			if(q.depth == 0)
				*out = Symbol{q.sym};
			else
				*out = Quoted{.sym = q.sym, .depth = q.depth -1};
			return true;
		}
		auto operator()(const Cons &c) const -> bool {
			Atom *fn = interp.mod.memory.alloc();

			if(!c.car->visit(Eval{.interp =interp, .self = *c.car, .out = fn})) return false;

			std::vector<Atom> args;

			Atom *current = c.cdr;
			while (auto *c = std::get_if<Cons>(current)) {
				args.push_back(nil{});
				if (!c->car->visit(Eval{.interp =interp, .self = *c->car, .out = &args.back()})) return false;
					
				current = c->cdr;
			}
			if (!std::holds_alternative<nil>(*current)) {
				interp.error = "Argument List has to be proper list!";
				return false;
			}
			*out = Call{.head = fn, .args = args};
			return true;
		}
		template<class T>
		auto operator()(const T &_) const -> bool {
			*out = self;
			return true;
		}
	};

	auto eval(Interpreter &interp, Env &called_from, Env &_, const std::vector<Atom> &args,
			  Atom *out) -> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}
		Atom evaled;
		if(!interp.eval(args[0], interp.mod.global, &evaled)) return false;
		evaled.visit(
					 Eval{.interp = interp, .self = evaled, .out = &evaled});
		return interp.eval(evaled, called_from, out);
	}

	auto define(Interpreter &interp, Env &called_from, Env &_, const std::vector<Atom> &args,
				Atom *out) -> bool {
		if (args.size() != 2) {
			interp.error = argument_error(2, args.size());
			return false;
		}
		if (const auto *sym = std::get_if<Symbol>(&args.at(0))) {
			Atom *cell = interp.mod.memory.alloc();
			if(!interp.eval(args[1], called_from, cell)) return false;
			called_from[sym->id] = cell;
			*out = *cell;
			return true;
		}
		interp.error = "First Argument must be a Symbol";
		return false;
	}
	auto lambda(Interpreter &interp, Env &called_from, Env &env,
				const std::vector<Atom> &args, Atom *out) -> bool {
		if (args.empty()) {
			interp.error = "Expected at least one Argument";
			return false;
		}
		if (const auto *arg_list = std::get_if<Call>(&args.at(0))) {
			std::vector<SymbolId> arglist;
			arglist.reserve(arg_list->args.size() + 1);

			
			auto get_sym = [&arglist, &interp](const Atom *a) {
				if (const auto *sym = std::get_if<Symbol>(a)) {
					arglist.push_back(sym->id);
					return true;
				}
				interp.error =
					"Lambda ArgumentList must only contain Symbols!";
				return false;
			};

			if(!get_sym(arg_list->head)) return false;
			for (const auto &a : arg_list->args)
			  if (!get_sym(&a))
			      return false;

			Atom *body = interp.mod.memory.alloc();
			*body = args[1];
			*out	   = Closure{.body = body,
					     .env  = called_from,
					     .args = std::move(arglist)};
			return true;
		}
		interp.error = "First Argument must be argument list";
		return false;
	}
	auto cons(Interpreter &interp, Env &called_from, Env &_,
			  const std::vector<Atom> &args, Atom *out) -> bool {
		if (args.size() != 2) {
			interp.error = argument_error(2, args.size());
			return false;
		}
		
		Atom *car = interp.mod.memory.alloc();
		if(!interp.eval(args[0], called_from, car)) return false;
		Atom *cdr = interp.mod.memory.alloc();
		if (!interp.eval(args[1], called_from, cdr))
			return false;
		*out = Cons{car, cdr};
		return true;
	}
	auto set(Interpreter &interp, Env &called_from, Env &_,
			 const std::vector<Atom> &args, Atom *out) -> bool {
		if (args.size() != 2) {
			interp.error = argument_error(2, args.size());
			return false;
		}
		if (const auto *sym = std::get_if<Symbol>(&args.at(0))) {
			*out = nil{};

			if (!called_from.contains(sym->id)) {
				interp.error = interp.mod.get_symbol_name(sym->id) + " is not defined yet!";
				return false;
			}
			return interp.eval(args[1], called_from,
						  called_from[sym->id]);
		}
		interp.error = "First Argument must be a Symbol";
		return false;
	}
	  
}
