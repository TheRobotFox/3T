#include "Types.hpp"

namespace TTT {

	struct Expr;
	struct Call {
		Expr *func;
		std::vector<Expr> args;
	};
	
	struct Expr {
		enum class Kind : uint8_t {
			ATOM,
			CALL,
		} kind;
		union {
			Heap_p val;
			Call call;
		};
	};	  
	
}
