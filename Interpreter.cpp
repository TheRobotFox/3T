#include "Interpreter.hpp"
#include <format>
#include <utility>
#include <variant>
#include <vector>

using namespace TTT;

auto next_arg(ConsExpr *current) -> bool
{
	auto *res = std::get_if<ConsExpr>(&(current)->cdr->body);
	if(res == nullptr) return false;
	*current = *res;
	return true;
}

auto Interpreter::eval_closure( Closure &cl, Environment& env, ConsExpr args, Sexp *out) -> bool
{
	int count = 0;
	for(SymbolExpr arg_name : cl.args){
		if(args.cdr == nullptr) goto arg_count_error;
		if(!next_arg(&args)) goto proper_list_error;
			
		Sexp *arg_val = mod.memory.alloc();
		if(!eval(*args.car, env, arg_val)) return false;
		cl.env.bind(arg_name, arg_val);
		count++;
	}

	if(cl.rest){
		Sexp *rest = mod.memory.alloc();
		while(args.cdr != nullptr){
			if(!next_arg(&args)) goto proper_list_error;
			Sexp *arg_val = mod.memory.alloc(), *next = mod.memory.alloc();
			rest->body = ConsExpr{.car=arg_val, .cdr=next};
			rest = next;

			if(!eval(*args.car, env, arg_val)) return false;
		}
		cl.env.bind(*cl.rest, rest);

	} else if(args.cdr != nullptr) goto arg_count_error;
		
	return eval(*cl.body, cl.env, out);

 arg_count_error:
	error = std::format("Unexpected Number of Arguments! Expected %d got %d!", cl.args.size(), count);
	return false;
	
 proper_list_error:
	error = std::format("Argument List not proper List!");
	return false;
}

auto Interpreter::eval_special( Special &fn, Environment &env, ConsExpr args, Sexp *out) -> bool
{
	return fn.func(args, env, out, &error);
}	 


auto Interpreter::eval(const Sexp &x, Environment &env, Sexp *out) -> bool
{
	if(const auto* sym = std::get_if<SymbolExpr>(&x.body)){
		
		if(auto *s = env.lookup(*sym)){
			*out = *s;
			return true;
		}
		error = std::format("Could not find Symbol %s in Environment!", mod.symbol_names[*sym]);
		return false;
	}
	if(const auto* cons = std::get_if<ConsExpr>(&x.body)){

		Sexp eval_fn;
		if(!eval(*cons->car, env, &eval_fn)) return false;

		if(auto *callable = std::get_if<CallableExpr>(&eval_fn.body)){
			if( auto *cl = std::get_if<Closure>(&callable->value))
				return eval_closure(*cl, env, *cons, out);
			if( auto *mc = std::get_if<Macro>(&callable->value))
				return false;
			if( auto *sp = std::get_if<Special>(&callable->value))
				return eval_special(*sp, env, *cons, out);
		}
		error = std::format("Form is not a callable!");
		return false;
	}
	*out = x;
	return true;
}	 

