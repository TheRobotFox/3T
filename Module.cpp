#include "Module.hpp"

#include <unordered_map>
#include <utility>
#include "Atom.hpp"
#include "Special.hpp"

using std::move;

using namespace TTT;



Module::Module()
: memory(1024 * 100) // 100Mb
{
	register_atom("cons?"		, Special{.func = is_T<Cons>		, .env = {}	});
	register_atom("num?"		, Special{.func = is_T<Real>		, .env = {}	});
	register_atom("int?"		, Special{.func = is_T<Integer>	, .env = {}	});
	register_atom("string?"	, Special{.func = is_T<String>		, .env = {}	});
	register_atom("symbol?"	, Special{.func = is_T<Symbol>		, .env = {}	});
	register_atom("quoted?"	, Special{.func = is_T<Quoted>		, .env = {}	});
	register_atom("hash?"		, Special{.func = is_T<HashTable>	, .env = {} });
	register_atom("in-port?"	, Special{.func = is_T<InPort>		, .env = {}	});
	register_atom("out-port?"	, Special{.func = is_T<OutPort>	, .env = {}	});
	register_atom("closure?"	, Special{.func = is_T<Closure>	, .env = {} });
	register_atom("car"		, Special{.func = get_car			, .env = {} });
	register_atom("cdr"		, Special{.func = get_cdr			, .env = {} });
	register_atom("if"			, Special{.func = _if				, .env = {} });
	f_read = register_atom("read"		, Special{.func = read				, .env = {} });
	f_error = register_atom("error"		, Special{.func = error			, .env = {} });
	
	init_readTable();
}

void Module::init_readTable() {

	Atom *eof_func = allocate(Closure{.body = allocate(Quoted{eof}), .env = {}, .args = {}});
	Atom *read_delims = register_atom(
	    "read-delimeters",
	    Special{.func = read_delimeter,
		    .env  = {
				{InternalSymbols::reader_eof_func, eof_func},
				{InternalSymbols::reader_backup_fun, nullptr}}});

	Atom *unbalanced_paren_error_func = allocate(Call{.head = f_error, .args = {String{"Unbalanced Parentethies!"}}});
	Atom *read_parenths =
	    allocate(Closure{.body = allocate(Call{.head = read_delims,
						   .args = {
						       Char{')'},
						       
						  Closure{.body = unbalanced_paren_error_func,
								  .env	= {},
								  .args = {}},
					  }}),
		.env	 = {}, .args = {}});

	Atom *unbalanced_brackets_error_func = allocate(Closure{.body = (Atom[]){Call{.head = f_error, .args = {String{"Unbalanced Brackets!"}}}}, .env = {}, .args = {}});
	Atom *read_brackets = allocate(Closure{
	    .body =
		allocate(Call{.head = read_delims,
					  .args = {
						  Char{')'},
						  Closure{.body = unbalanced_brackets_error_func,
								  .env	= {},
								  .args = {}},
					  }}),
		.env	 = {}, .args = {}});

	
	// Whitespace
	Atom	*f_read_whitespace = allocate(Special{.func = read_whitespace, .env = {}});
	Atom	*f_read_comment	   = allocate(Special{.func = read_comment, .env = {}});
	Atom	*f_read_string	   = allocate(Special{.func = read_string, .env = {}});
	Atom	*f_read_char	   = allocate(Special{.func = read_char, .env = {}});

	readTable = {{Char{')'}, unbalanced_paren_error_func},
				 {Char{'('}, read_parenths},
				 {Char{']'}, unbalanced_brackets_error_func},
				 {Char{'['}, read_brackets},
				 {Char{' '}, f_read_whitespace},
				 {Char{'\t'}, f_read_whitespace},
				 {Char{'\n'}, f_read_whitespace},
				 {Char{';'}, f_read_comment},
				 {Char{'"'}, f_read_string},
				 {Char{'?'}, f_read_char}};
	
	register_atom("*readtable*", HashTable(readTable));
}
