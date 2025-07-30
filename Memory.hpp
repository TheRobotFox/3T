#include <cstddef>
#include <vector>
#include "Expr.hpp"

namespace TTT {

	class Memory {

		float grow_factor = 2;
		size_t grow_threshold = 2 << 10;

		std::vector<Sexp> block;
	public:
		Memory(size_t initial) { block.reserve(initial); }

		/*
		 * Allocate new Cell initilized to null
		 */
		auto alloc() -> Sexp*;
		/*
		 * Perform compacting copy of Memory
		 */
		auto compact();
		void clear_marks();
		void mark();
	};
} // namespace TTT
