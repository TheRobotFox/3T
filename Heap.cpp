#include "Memory.hpp"

namespace TTT {

	auto Heap::alloc() -> Atom * { return reinterpret_cast<Atom*>(current++); }
	auto Heap::available() const -> size_t {return end-current;}

}
