#include "Module.hpp"
#include "Interpreter.hpp"
#include <format>
#include <functional>

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
	auto is_T(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool {
		if (args.size() != 1) {
			interp.error = argument_error(1, args.size());
			return false;
		}
		Atom *evaluated = interp.mod.memory.alloc();
		if (!interp.eval(args[0], env, evaluated))
			return false;
		
		if (std::holds_alternative<T>(*evaluated))
			*out = t{};
		else 
			*out = nil{};
		return true;
	}
	
	auto error(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool;
	
	auto get_car(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto get_cdr(Interpreter &interp, Env &env, const std::vector<Atom> &args,
				 Atom *out) -> bool;

	auto _if(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			 Atom *out) -> bool;

	auto read_delimeter		(Interpreter &interp, Env &env, const std::vector<Atom> &call, Atom *out) -> bool;
	auto read_whitespace	(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_comment		(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_string		(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read_char			(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;
	auto read				(Interpreter &interp, Env &env, const std::vector<Atom> &args, Atom *out) -> bool;

	auto eval(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool;
	auto list(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			  Atom *out) -> bool;
	auto quote(Interpreter &interp, Env &env, const std::vector<Atom> &args,
			   Atom *out) -> bool;
}
