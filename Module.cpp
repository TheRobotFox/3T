#include "Module.hpp"
#include "Atom.hpp"
#include "Special.hpp"

using namespace TTT;

Module::Module()
: memory(1024 * 1024 * 100) // 100Mb
{

	// Read Table
	// Initilize Global Scope
	Atom	*isCons		= memory.alloc();
	*isCons				= Special{.env = {}, .func = is_T<Cons>		};
	Atom	*isReal		= memory.alloc();
	*isReal				= Special{.env = {}, .func = is_T<Cons>		};
	Atom	*isInteger	= memory.alloc();
	*isInteger			= Special{.env = {}, .func = is_T<Real>		};
	Atom	*isString	= memory.alloc();
	*isString			= Special{.env = {}, .func = is_T<Integer>	};
	Atom	*isSymbol	= memory.alloc();
	*isSymbol			= Special{.env = {}, .func = is_T<String>	};
	Atom	*isQuoted	= memory.alloc();
	*isQuoted			= Special{.env = {}, .func = is_T<Symbol>	};
	Atom	*isHashTable= memory.alloc();
	*isHashTable		= Special{.env = {}, .func = is_T<Quoted>	};
	Atom	*isInPort	= memory.alloc();
	*isInPort			= Special{.env = {}, .func = is_T<HashTable>};
	Atom	*isOutPort	= memory.alloc();
	*isOutPort			= Special{.env = {}, .func = is_T<InPort>	};
	Atom	*isClosure	= memory.alloc();
	*isClosure			= Special{.env = {}, .func = is_T<OutPort>	};
	Atom	*car	 	= memory.alloc();
	*car			 	= Special{.env = {}, .func = get_car		};
	Atom	*cdr	 	= memory.alloc();
	*cdr			 	= Special{.env = {}, .func = get_cdr		};
	Atom	*__if 	 	= memory.alloc();
	*__if				= Special{.env = {}, .func = _if			};
	Atom	*_error 	 	= memory.alloc();
	*_error				= Special{.env = {}, .func = error			};

	

	
	Atom *eof_func = memory.alloc();
	*eof_func      = Closure{.body = (Atom[]){Symbol{eof}}};

	// Read ()

	Atom *closing = memory.alloc();
	*closing = Char{')'};

	Atom *unbalanced_paren_error_func = memory.alloc();
	*unbalanced_paren_error_func = Closure{.body = (Atom[]){Call{.head = _error, .args = {String{"Unbalanced Parentethies!"}}}}};

	Atom *read_parenths = memory.alloc();
	*read_parenths	    = Special {
	  .func = read_delimeter, .env = {
	      {intern("closing"), closing},
	      {intern("eof-func"), eof_func}
	}
	};
	
	readtable		 = memory.alloc();
	*readtable		 = HashTable {
	  .value = {
	    {Char{')'}, unbalanced_paren_error_func},
	    {Char{'('}, read_parenths},
	    {Char{}}
		}};
	


	global = {
		{intern("cons?"	 )	, isCons},
		{intern("real?"	 )	, isReal},
		{intern("int?"	 )	, isInteger},
		{intern("string?"	 )	, isString},
		{intern("symbol?"	 )	, isSymbol},
		{intern("quoted?"	 )	, isQuoted},
		{intern("hash?"	 )	, isHashTable},
		{intern("in-port?" )	, isInPort},
		{intern("out-port?")	, isOutPort},
		{intern("closure?" )	, isClosure},
		
		{intern("car"	 )	, car},
		{intern("cdr"	 )	, cdr},
		{intern("if" 	 )	, __if},

		{intern("*readtable*"), readtable},
		{intern("*in-port*"		), in_port},
		{intern("*out-port*"), out_port},
	};
}
