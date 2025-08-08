#include "Memory.hpp"
#include "Atom.hpp"
#include <cstddef>
#include <vector>

namespace TTT {
	auto Memory::alloc() -> Atom * { return heap.get() + (current++); }
	void Memory::mark(Atom *_){}
}
