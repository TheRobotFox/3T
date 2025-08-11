#include "Module.hpp"
#include "Interpreter.hpp"
#include "Printer.hpp"

#include <format>
#include <functional>
#include <sstream>

namespace TTT {

	//	template <class T>
	//	constexpr
	//	std::string_view
	//	type_name()
	//	{
	//		using namespace std;
	// #ifdef __clang__
	//		string_view p = __PRETTY_FUNCTION__;
	//		return string_view(p.data() + 34, p.size() - 34 - 1);
	// #elif defined(__GNUC__)
	//		string_view p = __PRETTY_FUNCTION__;
	// #  if __cplusplus < 201402
	//		return string_view(p.data() + 36, p.size() - 36 - 1);
	// #  else
	//		return string_view(p.data() + 49, p.find(';', 49) - 49);
	// #  endif
	// #elif defined(_MSC_VER)
	//		string_view p = __FUNCSIG__;
	//		return string_view(p.data() + 84, p.size() - 84 - 7);
	// #endif
	//	}


	//	template <class Fn, class... Args>
	//	struct arg_parser {

	//		std::string error;
	//		template<std::size_t I = 0, typename... Tp>
	//		inline typename std::enable_if<I == sizeof...(Tp), void>::type
	//		parse_arg(std::tuple<Tp...>& t, Sexp *null)
	//		{
	//			if(null == nullptr || std::holds_alternative<Null>(null->body))
	//				return true;
	//			error = std::format("Wrong Number of Arguments! Expected %d got >%d", sizeof...(Tp), I);
	//			return false;
	//		}

	//		template<std::size_t I = 0, typename... Tp>
	//		inline typename std::enable_if<I < sizeof...(Tp), void>::type
	//		parse_arg(std::tuple<Tp&...> &t, Sexp *sexp)
	//		{
	//			if(const auto *argv = std::get_if<ConsExpr>(&sexp->body)){
			
	//				if(const auto *arg = std::get_if<Tp...[I]>(&argv->car->body)){
	//					std::get<I>(t) = *arg;
	//					return parse_arg<I+1, Tp...>(t, argv->cdr);
	//				}
	//				error = std::format("Arg %d wrong Type! Expected %s got %d", type_name<Tp...[I]>(), argv->car->body.index(), I);
	//				return false;
	//			}
	//			if(std::holds_alternative<Null>(sexp->body))
	//				error = std::format("Wrong Number of Arguments! Expected %d got %d", I, sizeof...(Tp));
	//			else
	//				error = std::format("Arglist not proper List at %d! Expected Cons got %d", I, sexp->body.index());
	//			return false;
	//		}


	//		auto operator()() -> std::function<bool(Interpreter &interpreter, ConsExpr sexp, Env &env,
	//						Sexp *out)>
	//		{
	//			return [*this](Interpreter &interpreter, ConsExpr sexp, Env &env, Sexp *out){
	//				std::tuple<Args&...> argv;
	//				parse_arg<0, Args...>(argv, sexp.cdr);

	//				std::apply(Fn{},
	// std::tuple_cat(std::make_tuple(interpreter, env, out), argv));
	//			};
	//		}
	//	};

	enum InternalSymbols : SymbolId {
		eof				  = -1,
		reader_backup_fun = -2,
		reader_eof_func	  = -3,
	};

	auto argument_error(int expected, int got) -> std::string;


	template <class T>
	auto is_T(Interpreter &interp, Env &called_from, Env &_, const std::vector<Atom> &args,
			  Atom *out) -> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}
		Atom *evaluated = interp.mod.memory.alloc();
		if (!interp.eval(args[0], called_from, evaluated))
			return false;
		
		if (std::holds_alternative<T>(*evaluated))
			*out = t{};
		else 
			*out = nil{};
		return true;
	}
	
	auto error(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool;
	
	auto get_car(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto get_cdr(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
				 Atom *out) -> bool;

	auto _if(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			 Atom *out) -> bool;

	auto read_delimeter		(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &call, Atom *out) -> bool;
	auto read_whitespace	(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_comment		(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_string		(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_char			(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read				(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;

	auto eval(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool;
	auto list(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool;
	auto quote(Interpreter &interp, Env &_, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool;
	auto define(Interpreter &interp, Env &_, Env &env,
				const std::vector<Atom> &args, Atom *out) -> bool;
	auto lambda(Interpreter &interp, Env &_, Env &env,
				const std::vector<Atom> &args, Atom *out) -> bool;
	auto cons(Interpreter &interp, Env &_, Env &env,
			  const std::vector<Atom> &args, Atom *out) -> bool;
	auto set(Interpreter &interp, Env &called_from, Env &_,
			 const std::vector<Atom> &args, Atom *out) -> bool;


	template <class Op>
	auto make_op(Op &&fn, int neutral) // FIXME Divison by zero
	    -> std::function<bool(Interpreter &, Env &, Env &,
						   const std::vector<Atom> &, Atom *)> {
		return [fn, neutral](Interpreter &interp, Env &called_from, Env &_,
		     const std::vector<Atom> &args, Atom *out) -> bool {
	      long long ires = neutral;
	      double rres    = neutral;
	      bool exact = true;
			for (const Atom &a : args) {
				Atom evaled;
				if (!interp.eval(a, called_from, &evaled))
					return false;
				if (const auto *i =
				std::get_if<Integer>(&evaled)) {
					ires = fn(ires, i->value);
					rres = fn(rres, i->value);
				} else if (const auto *r =
				std::get_if<Real>(&evaled)) {
					rres = fn(rres, r->value);
					exact = false;
				} else {
					std::ostringstream os;
					os << "Operator only accepts Number inputs! Got ";
					evaled.visit(Printer(os, &interp.mod));
					interp.error = os.str();
					return false;
				}
			}
			if (exact)
				*out = Integer{.value = ires};
			else
				*out = Real{.value = rres};

			return true;
		};
	}
	
}
