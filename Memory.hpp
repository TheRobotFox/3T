#include <cstddef>
#include <vector>
#include "Atom.hpp"

namespace TTT {

	struct Memory {
		auto alloc() -> Atom*;
		void collect();
		void mark(Atom *);
	};
	
} // namespace TTT
